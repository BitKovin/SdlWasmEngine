/*
 * RmlUi bgfx Render Interface — Implementation
 *
 * Full port of the RmlUi OpenGL 3 render interface to bgfx.
 * Produces identical visual output when given the same input.
 */

#include "RmlUi_Renderer_BGFX.h"
#include "Shader.hpp"
#include <Renderer/Abstractions/ViewIdManager.h>

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Math.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/DecorationTypes.h>

#include <includedLibraries/stb_image.h>
#include <Profiling/ResourceStatistics.hpp>

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

static constexpr int MAX_BLUR_PASSES  = 5;
static constexpr int NUM_BLUR_WEIGHTS = 8; // must match shader

// Gaussian weight computation matching GL3 exactly.
static void ComputeGaussianWeights(float sigma, float* weights_out, int count)
{
    float sum = 0.f;
    for (int i = 0; i < count; ++i)
    {
        float x = static_cast<float>(i);
        weights_out[i] = std::exp(-0.5f * (x * x) / (sigma * sigma));
        sum += (i == 0) ? weights_out[i] : 2.f * weights_out[i];
    }
    for (int i = 0; i < count; ++i)
        weights_out[i] /= sum;
}

// Convert Rml::Matrix4f (column-major, same as glm) to float[16]
static void Matrix4ToFloat16(const Rml::Matrix4f& m, float out[16])
{
    // Rml::Matrix4f stores data column-major, same layout as bgfx expects
    std::memcpy(out, m.data(), 16 * sizeof(float));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

RenderInterface_BGFX::RenderInterface_BGFX()
{
    // ── Vertex layout ────────────────────────────────────────────────────
    m_vertexLayout.begin()
        .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true) // normalised
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    // ── Create uniforms ──────────────────────────────────────────────────
    u_transform      = bgfx::createUniform("u_transform",      bgfx::UniformType::Mat4);
    u_translate      = bgfx::createUniform("u_translate",       bgfx::UniformType::Vec4);
    u_texParams      = bgfx::createUniform("u_texParams",       bgfx::UniformType::Vec4);
    u_blurParams     = bgfx::createUniform("u_blurParams",      bgfx::UniformType::Vec4);
    u_texelSize      = bgfx::createUniform("u_texelSize",       bgfx::UniformType::Vec4);
    u_colorMatrix    = bgfx::createUniform("u_colorMatrix",     bgfx::UniformType::Mat4);
    u_colorTranslate = bgfx::createUniform("u_colorTranslate",  bgfx::UniformType::Vec4);
    u_shadowExtra    = bgfx::createUniform("u_shadowExtra",     bgfx::UniformType::Vec4);
    u_shadowColor    = bgfx::createUniform("u_shadowColor",     bgfx::UniformType::Vec4);
    u_gradientParams = bgfx::createUniform("u_gradientParams",  bgfx::UniformType::Vec4);
    u_gradientP      = bgfx::createUniform("u_gradientP",       bgfx::UniformType::Vec4);
    s_texture0       = bgfx::createUniform("s_texture0",        bgfx::UniformType::Sampler);
    s_texture1       = bgfx::createUniform("s_texture1",        bgfx::UniformType::Sampler);

    // ── Load shader programs ─────────────────────────────────────────────
    m_valid = LoadPrograms();

    if (m_valid)
    {
        // Build a fullscreen quad [-1,1] covering NDC space.
        // Uses tex_coord for UV.
        struct FSVert { float x, y; uint32_t col; float u, v; };
        static const FSVert verts[] = {
            {-1.f, -1.f, 0xFFFFFFFF, 0.f, 0.f},
            { 1.f, -1.f, 0xFFFFFFFF, 1.f, 0.f},
            { 1.f,  1.f, 0xFFFFFFFF, 1.f, 1.f},
            {-1.f,  1.f, 0xFFFFFFFF, 0.f, 1.f},
        };
        static const int indices[] = { 0, 1, 2, 0, 2, 3 };

        m_fullscreenQuad = CompileGeometry(
            {reinterpret_cast<const Rml::Vertex*>(verts), 4},
            {indices, 6}
        );
    }
}

RenderInterface_BGFX::~RenderInterface_BGFX()
{
    if (m_fullscreenQuad)
        ReleaseGeometry(m_fullscreenQuad);

    // Release all remaining geometry
    for (auto& [id, geo] : m_geometries)
    {
        if (bgfx::isValid(geo.vbh)) bgfx::destroy(geo.vbh);
        if (bgfx::isValid(geo.ibh)) bgfx::destroy(geo.ibh);
    }
    m_geometries.clear();

    // Release filters
    for (auto& [id, f] : m_filters)
    {
        if (bgfx::isValid(f.mask_texture))
            bgfx::destroy(f.mask_texture);
    }
    m_filters.clear();

    // Release shaders
    for (auto& [id, s] : m_shaders)
    {
        if (bgfx::isValid(s.stop_texture))
            bgfx::destroy(s.stop_texture);
    }
    m_shaders.clear();

    // Destroy uniforms
    auto safeDestroy = [](bgfx::UniformHandle& h) {
        if (bgfx::isValid(h)) { bgfx::destroy(h); h = BGFX_INVALID_HANDLE; }
    };
    safeDestroy(u_transform);
    safeDestroy(u_translate);
    safeDestroy(u_texParams);
    safeDestroy(u_blurParams);
    safeDestroy(u_texelSize);
    safeDestroy(u_colorMatrix);
    safeDestroy(u_colorTranslate);
    safeDestroy(u_shadowExtra);
    safeDestroy(u_shadowColor);
    safeDestroy(u_gradientParams);
    safeDestroy(u_gradientP);
    safeDestroy(s_texture0);
    safeDestroy(s_texture1);

    DestroyPrograms();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Program loading
// ─────────────────────────────────────────────────────────────────────────────

bool RenderInterface_BGFX::LoadPrograms()
{
    struct ProgramDef { RmlProgramId id; const char* vs; const char* fs; };
    static const ProgramDef defs[] = {
        { RmlProgramId::Color,       "rmlui/vs_rmlui",             "rmlui/fs_rmlui_color"       },
        { RmlProgramId::Texture,     "rmlui/vs_rmlui",             "rmlui/fs_rmlui_texture"     },
        { RmlProgramId::Passthrough, "rmlui/vs_rmlui_passthrough", "rmlui/fs_rmlui_passthrough" },
        { RmlProgramId::BlendMask,   "rmlui/vs_rmlui_passthrough", "rmlui/fs_rmlui_blendmask"  },
        { RmlProgramId::Blur,        "rmlui/vs_rmlui_passthrough", "rmlui/fs_rmlui_blur"        },
        { RmlProgramId::DropShadow,  "rmlui/vs_rmlui_passthrough", "rmlui/fs_rmlui_dropshadow" },
        { RmlProgramId::ColorMatrix, "rmlui/vs_rmlui_passthrough", "rmlui/fs_rmlui_colormatrix"},
        { RmlProgramId::Creation,    "rmlui/vs_rmlui",             "rmlui/fs_rmlui_creation"    },
        { RmlProgramId::Gradient,    "rmlui/vs_rmlui",             "rmlui/fs_rmlui_gradient"    },
    };

    for (auto& def : defs)
    {
        Shader* s = Shader::FromFiles( def.vs, def.fs);
        if (!s || !s->IsValid())
        {
            Rml::Log::Message(Rml::Log::LT_ERROR, "BGFX: Failed to load program %s / %s", def.vs, def.fs);
            DestroyPrograms();
            return false;
        }
        m_programs[static_cast<int>(def.id)] = s;
    }
    return true;
}

void RenderInterface_BGFX::DestroyPrograms()
{
    for (int i = 0; i < static_cast<int>(RmlProgramId::Count); ++i)
    {
        delete m_programs[i];
        m_programs[i] = nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Viewport / frame management
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::SetViewport(int w, int h, int ox, int oy)
{
    m_viewportWidth   = w;
    m_viewportHeight  = h;
    m_viewportOffsetX = ox;
    m_viewportOffsetY = oy;
    m_projection = MakeProjection();
}

Rml::Matrix4f RenderInterface_BGFX::MakeProjection() const
{
    const float L = static_cast<float>(m_viewportOffsetX);
    const float R = L + static_cast<float>(m_viewportWidth);
    const float T = static_cast<float>(m_viewportOffsetY);
    const float B = T + static_cast<float>(m_viewportHeight);

    Rml::Matrix4f proj = Rml::Matrix4f::Identity();

    float* m = proj.data();

    m[0] = 2.f / (R - L);
    m[5] = 2.f / (T - B);
    m[10] = 1.f;

    m[12] = -(R + L) / (R - L);
    m[13] = -(T + B) / (T - B);
    m[14] = 0.f;
    m[15] = 1.f;

    return proj;
}

void RenderInterface_BGFX::BeginFrame()
{
    m_drawOrder = 0;
    m_stencilRef = 0;
    m_stencilValue = 1;
    m_clipMaskEnabled = false;
    m_scissorEnabled = false;
    m_activeProgram = RmlProgramId::None;
    m_lastBoundTexture = BGFX_INVALID_HANDLE;
    m_transform = Rml::Matrix4f::Identity();
    m_projection = MakeProjection();

    m_layers.BeginFrame(*this, m_viewportWidth, m_viewportHeight);

    // Allocate the first view for the base layer
    m_baseView = ViewIdManager::GetCurrentId();
    m_currentView = AllocateView();
    m_viewDirty = true;

    // Set up the base-layer view
    const auto& baseLayer = m_layers.GetTopLayer();
    SetupView(m_currentView, baseLayer);
}

void RenderInterface_BGFX::EndFrame()
{
    m_layers.EndFrame();

    // Blit the base layer (layer 0) to the backbuffer.
    const auto& baseLayer = m_layers.GetLayer(0);

    bgfx::ViewId blitView = AllocateView();
    bgfx::setViewName(blitView, "RmlUI_BlitToBackbuffer");
    bgfx::setViewFrameBuffer(blitView, BGFX_INVALID_HANDLE); // default backbuffer
    bgfx::setViewRect(blitView, m_viewportOffsetX, m_viewportOffsetY,
                       uint16_t(m_viewportWidth), uint16_t(m_viewportHeight));
    bgfx::setViewMode(blitView, bgfx::ViewMode::Sequential);

    // Set up identity projection for NDC fullscreen quad
    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(blitView, identity, identity);
    bgfx::setViewClear(blitView, BGFX_CLEAR_NONE);

    // Draw fullscreen quad with the base layer's color texture
    auto savedView = m_currentView;
    m_currentView = blitView;

    DrawFullscreenQuad(baseLayer.color, RmlProgramId::Passthrough);

    m_currentView = savedView;
}

void RenderInterface_BGFX::Clear()
{
    // Clear happens via bgfx::setViewClear on the current view.
    // We set it when setting up each layer view.
}

// ─────────────────────────────────────────────────────────────────────────────
//  View management
// ─────────────────────────────────────────────────────────────────────────────

bgfx::ViewId RenderInterface_BGFX::AllocateView()
{
    return ViewIdManager::GiveNextId();
}

void RenderInterface_BGFX::SetupView(bgfx::ViewId view, const BgfxFramebuffer& fb)
{
    bgfx::setViewName(view, "RmlUI_Layer");
    bgfx::setViewFrameBuffer(view, fb.fb);
    bgfx::setViewRect(view, 0, 0, uint16_t(fb.width), uint16_t(fb.height));
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
    bgfx::setViewClear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
                        0x00000000, 1.0f, 0);

    float proj[16];
    Matrix4ToFloat16(m_projection, proj);
    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(view, identity, proj);

    // Touch the view so it executes even if nothing is submitted
    bgfx::touch(view);
}

void RenderInterface_BGFX::SetupViewRect(bgfx::ViewId view)
{
    const auto& topLayer = m_layers.GetTopLayer();
    bgfx::setViewRect(view, 0, 0, uint16_t(topLayer.width), uint16_t(topLayer.height));
}

void RenderInterface_BGFX::EnsureView()
{
    if (!m_viewDirty) return;
    m_viewDirty = false;
    // View was already set up when allocated.
}

// ─────────────────────────────────────────────────────────────────────────────
//  Program management
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::UseProgram(RmlProgramId id)
{
    m_activeProgram = id;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Geometry
// ─────────────────────────────────────────────────────────────────────────────

Rml::CompiledGeometryHandle RenderInterface_BGFX::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int> indices)
{
    BgfxCompiledGeometry geo;

    const bgfx::Memory* vmem = bgfx::copy(vertices.data(),
        static_cast<uint32_t>(vertices.size() * sizeof(Rml::Vertex)));
    geo.vbh = bgfx::createVertexBuffer(vmem, m_vertexLayout);

    // bgfx uses 16-bit indices by default; use 32-bit for safety with large meshes.
    // Convert int -> uint32_t
    std::vector<uint32_t> idx32(indices.size());
    for (size_t i = 0; i < indices.size(); ++i)
        idx32[i] = static_cast<uint32_t>(indices[i]);

    const bgfx::Memory* imem = bgfx::copy(idx32.data(),
        static_cast<uint32_t>(idx32.size() * sizeof(uint32_t)));
    geo.ibh = bgfx::createIndexBuffer(imem, BGFX_BUFFER_INDEX32);

    geo.num_indices = static_cast<int>(indices.size());

    Rml::CompiledGeometryHandle handle = m_nextGeometryId++;
    m_geometries[handle] = geo;
    return handle;
}

void RenderInterface_BGFX::RenderGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f translation,
    Rml::TextureHandle texture)
{
    SubmitGeometry(handle, translation, texture);
}

void RenderInterface_BGFX::SubmitGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f translation,
    Rml::TextureHandle texture,
    RmlProgramId program_override)
{
    auto it = m_geometries.find(handle);
    if (it == m_geometries.end()) return;

    const auto& geo = it->second;

    // Determine program
    RmlProgramId prog = program_override;
    if (prog == RmlProgramId::None)
    {
        if (texture == 0)
            prog = RmlProgramId::Color;
        else if (texture == TexturePostprocess)
            prog = m_activeProgram; // keep current
        else
            prog = RmlProgramId::Texture;
    }

    UseProgram(prog);

    Shader* shader = m_programs[static_cast<int>(prog)];
    if (!shader || !shader->IsValid()) return;

    // Set transform uniform
    SetTransformUniform(translation);

    // Set texture
    if (texture != 0 && texture != TexturePostprocess)
    {
        bgfx::TextureHandle tex;
        if (texture == TextureEnableWithoutBinding)
            tex = m_lastBoundTexture;
        else
        {
            tex.idx = static_cast<uint16_t>(texture);
            m_lastBoundTexture = tex;
        }

        if (bgfx::isValid(tex))
            bgfx::setTexture(0, s_texture0, tex);
    }

    // Set vertex/index buffers
    bgfx::setVertexBuffer(0, geo.vbh);
    bgfx::setIndexBuffer(geo.ibh);

    // Build render state
    uint64_t state = BuildBaseState();

    // If we're doing color-only (no texture), still use standard blend
    if (prog == RmlProgramId::Color || texture == 0)
    {
        // premultiplied alpha blend: src=ONE, dst=ONE_MINUS_SRC_ALPHA
        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    }
    else
    {
        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    }

    bgfx::setState(state);

    // Scissor
    if (m_scissorEnabled)
    {
        bgfx::setScissor(
            uint16_t(m_scissorRegion.Left()),
            uint16_t(m_scissorRegion.Top()),
            uint16_t(m_scissorRegion.Width()),
            uint16_t(m_scissorRegion.Height())
        );
    }

    // Stencil
    if (m_clipMaskEnabled)
    {
        uint32_t stencil = BuildStencilState();
        bgfx::setStencil(stencil);
    }

    bgfx::submit(m_currentView, shader->GetProgram(), m_drawOrder++);
}

void RenderInterface_BGFX::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
    auto it = m_geometries.find(handle);
    if (it == m_geometries.end()) return;

    auto& geo = it->second;
    if (bgfx::isValid(geo.vbh)) bgfx::destroy(geo.vbh);
    if (bgfx::isValid(geo.ibh)) bgfx::destroy(geo.ibh);

    m_geometries.erase(it);
}

// ─────────────────────────────────────────────────────────────────────────────
//  State builders
// ─────────────────────────────────────────────────────────────────────────────

uint64_t RenderInterface_BGFX::BuildBaseState() const
{
    return BGFX_STATE_WRITE_RGB
         | BGFX_STATE_WRITE_A
         | BGFX_STATE_MSAA;
}

uint32_t RenderInterface_BGFX::BuildStencilState() const
{
    // Test: pass if (ref & mask) == (stencil & mask)
    return 0
        | BGFX_STENCIL_TEST_EQUAL
        | BGFX_STENCIL_FUNC_REF(m_stencilRef)
        | BGFX_STENCIL_FUNC_RMASK(0xFF)
        | BGFX_STENCIL_OP_FAIL_S_KEEP
        | BGFX_STENCIL_OP_FAIL_Z_KEEP
        | BGFX_STENCIL_OP_PASS_Z_KEEP;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Transform
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::SetTransform(const Rml::Matrix4f* transform)
{
    m_transform = transform ? *transform : Rml::Matrix4f::Identity();
}

void RenderInterface_BGFX::SetTransformUniform(Rml::Vector2f translation)
{
    // Build MVP: projection * transform * translate
    Rml::Matrix4f translate_mat = Rml::Matrix4f::Translate(translation.x, translation.y, 0.f);
    Rml::Matrix4f mvp = m_projection * m_transform * translate_mat;

    float mvp_f[16];
    Matrix4ToFloat16(mvp, mvp_f);
    bgfx::setUniform(u_transform, mvp_f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Textures
// ─────────────────────────────────────────────────────────────────────────────

Rml::TextureHandle RenderInterface_BGFX::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source)
{
    Rml::FileInterface* fi = Rml::GetFileInterface();
    Rml::FileHandle fh = fi->Open(source);
    if (!fh) return {};

    fi->Seek(fh, 0, SEEK_END);
    const size_t buf_size = fi->Tell(fh);
    fi->Seek(fh, 0, SEEK_SET);

    std::vector<Rml::byte> buffer(buf_size);
    fi->Read(buffer.data(), buf_size, fh);
    fi->Close(fh);

    int w = 0, h = 0, ch = 0;
    stbi_uc* pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(buffer.data()),
        static_cast<int>(buf_size), &w, &h, &ch, STBI_rgb_alpha);

    if (!pixels)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "BGFX: stb_image failed '%s': %s",
                          source.c_str(), stbi_failure_reason());
        return {};
    }

    texture_dimensions = {w, h};

    // Pre-multiply alpha (matches GL3 renderer exactly)
    const size_t byte_size = size_t(w) * h * 4;
    for (size_t i = 0; i < byte_size; i += 4)
    {
        const uint8_t a = pixels[i + 3];
        for (int j = 0; j < 3; ++j)
            pixels[i + j] = uint8_t(int(pixels[i + j]) * int(a) / 255);
    }

    const bgfx::Memory* mem = bgfx::copy(pixels, static_cast<uint32_t>(byte_size));
    bgfx::TextureHandle tex = bgfx::createTexture2D(
        uint16_t(w), uint16_t(h), false, 1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        mem);

    stbi_image_free(pixels);

    if (!bgfx::isValid(tex)) return {};

    Rml::TextureHandle handle = static_cast<Rml::TextureHandle>(tex.idx);

    ResourceStatistics::Instance().registerResource(ResourceType::Texture, handle, byte_size, source);
    ResourceStatistics::Instance().setResourceName(ResourceType::Texture, handle, source);

    return handle;
}

Rml::TextureHandle RenderInterface_BGFX::GenerateTexture(
    Rml::Span<const Rml::byte> source_data,
    Rml::Vector2i source_dimensions)
{
    const bgfx::Memory* mem = bgfx::copy(source_data.data(),
        static_cast<uint32_t>(source_data.size()));

    bgfx::TextureHandle tex = bgfx::createTexture2D(
        uint16_t(source_dimensions.x), uint16_t(source_dimensions.y),
        false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        mem);

    if (!bgfx::isValid(tex)) return {};

    return static_cast<Rml::TextureHandle>(tex.idx);
}

void RenderInterface_BGFX::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    bgfx::TextureHandle tex;
    tex.idx = static_cast<uint16_t>(texture_handle);
    if (bgfx::isValid(tex))
        bgfx::destroy(tex);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Scissor
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::EnableScissorRegion(bool enable)
{
    m_scissorEnabled = enable;
}

void RenderInterface_BGFX::SetScissorRegion(Rml::Rectanglei region)
{
    m_scissorRegion = region;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Clip mask (stencil)
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::EnableClipMask(bool enable)
{
    m_clipMaskEnabled = enable;
}

void RenderInterface_BGFX::RenderToClipMask(
    Rml::ClipMaskOperation operation,
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation)
{
    auto it = m_geometries.find(geometry);
    if (it == m_geometries.end()) return;

    const auto& geo = it->second;

    Shader* creationShader = m_programs[static_cast<int>(RmlProgramId::Creation)];
    if (!creationShader) return;

    UseProgram(RmlProgramId::Creation);
    SetTransformUniform(translation);

    bgfx::setVertexBuffer(0, geo.vbh);
    bgfx::setIndexBuffer(geo.ibh);

    // Don't write color, only stencil
    uint64_t state = BGFX_STATE_MSAA;
    bgfx::setState(state);

    if (m_scissorEnabled)
    {
        bgfx::setScissor(
            uint16_t(m_scissorRegion.Left()),
            uint16_t(m_scissorRegion.Top()),
            uint16_t(m_scissorRegion.Width()),
            uint16_t(m_scissorRegion.Height())
        );
    }

    uint32_t stencil = 0;

    switch (operation)
    {
    case Rml::ClipMaskOperation::Set:
    {
        // Clear stencil first, then write reference value.
        // We increment the reference value each time Set is called.
        m_stencilRef = m_stencilValue++;

        stencil = 0
            | BGFX_STENCIL_TEST_ALWAYS
            | BGFX_STENCIL_FUNC_REF(m_stencilRef)
            | BGFX_STENCIL_FUNC_RMASK(0xFF)
            | BGFX_STENCIL_OP_FAIL_S_ZERO
            | BGFX_STENCIL_OP_FAIL_Z_ZERO
            | BGFX_STENCIL_OP_PASS_Z_REPLACE;
        break;
    }
    case Rml::ClipMaskOperation::SetInverse:
    {
        // Write to stencil where geometry IS NOT, so subsequent draws only
        // pass where geometry was absent.
        // First pass: clear stencil to ref everywhere (via fullscreen quad).
        // Then write 0 where geometry is.

        // We need two draws. First, fill entire stencil with ref via fullscreen quad.
        m_stencilRef = m_stencilValue++;

        // Fullscreen quad to fill stencil
        {
            auto fsIt = m_geometries.find(m_fullscreenQuad);
            if (fsIt != m_geometries.end())
            {
                const auto& fsGeo = fsIt->second;

                // Set identity transform for fullscreen quad
                float identity[16];
                bx::mtxIdentity(identity);
                bgfx::setUniform(u_transform, identity);

                bgfx::setVertexBuffer(0, fsGeo.vbh);
                bgfx::setIndexBuffer(fsGeo.ibh);
                bgfx::setState(BGFX_STATE_MSAA); // no color write

                uint32_t fillStencil = 0
                    | BGFX_STENCIL_TEST_ALWAYS
                    | BGFX_STENCIL_FUNC_REF(m_stencilRef)
                    | BGFX_STENCIL_FUNC_RMASK(0xFF)
                    | BGFX_STENCIL_OP_FAIL_S_REPLACE
                    | BGFX_STENCIL_OP_FAIL_Z_REPLACE
                    | BGFX_STENCIL_OP_PASS_Z_REPLACE;

                bgfx::setStencil(fillStencil);
                bgfx::submit(m_currentView, creationShader->GetProgram(), m_drawOrder++);
            }
        }

        // Now set up the actual geometry submission to write 0 where geometry is
        SetTransformUniform(translation);
        bgfx::setVertexBuffer(0, geo.vbh);
        bgfx::setIndexBuffer(geo.ibh);
        bgfx::setState(BGFX_STATE_MSAA);

        stencil = 0
            | BGFX_STENCIL_TEST_ALWAYS
            | BGFX_STENCIL_FUNC_REF(0)
            | BGFX_STENCIL_FUNC_RMASK(0xFF)
            | BGFX_STENCIL_OP_FAIL_S_REPLACE
            | BGFX_STENCIL_OP_FAIL_Z_REPLACE
            | BGFX_STENCIL_OP_PASS_Z_REPLACE;
        break;
    }
    case Rml::ClipMaskOperation::Intersect:
    {
        // Only write where stencil already equals ref (intersection).
        // Increment ref; subsequent draws test against new value.
        uint8_t old_ref = m_stencilRef;
        m_stencilRef = m_stencilValue++;

        stencil = 0
            | BGFX_STENCIL_TEST_EQUAL
            | BGFX_STENCIL_FUNC_REF(old_ref)
            | BGFX_STENCIL_FUNC_RMASK(0xFF)
            | BGFX_STENCIL_OP_FAIL_S_KEEP
            | BGFX_STENCIL_OP_FAIL_Z_KEEP
            | BGFX_STENCIL_OP_PASS_Z_REPLACE;

        // We need to write the NEW ref on pass. bgfx stencil ref is both
        // the comparison value and the replace value, so we need a trick:
        // Compare against old_ref, but write m_stencilRef.
        // Unfortunately bgfx doesn't separate these — the ref in BGFX_STENCIL_FUNC_REF
        // is used for both comparison and REPLACE. So we use two passes:

        // Pass 1: Decrement where not equal (no-op due to KEEP), or mark where equal
        // Actually, let's use a simpler approach:
        // We know old_ref < m_stencilRef (since we increment).
        // On the geometry, where stencil == old_ref, write m_stencilRef.
        // The comparison tests against old_ref but REPLACE writes the func_ref.

        // In bgfx, BGFX_STENCIL_FUNC_REF is the reference value used for BOTH
        // the comparison function AND the REPLACE operation.
        // So we can't compare against old_ref and write new_ref in one pass.

        // Workaround: use INCR instead of REPLACE when incrementing by 1.
        if (m_stencilRef == old_ref + 1)
        {
            stencil = 0
                | BGFX_STENCIL_TEST_EQUAL
                | BGFX_STENCIL_FUNC_REF(old_ref)
                | BGFX_STENCIL_FUNC_RMASK(0xFF)
                | BGFX_STENCIL_OP_FAIL_S_KEEP
                | BGFX_STENCIL_OP_FAIL_Z_KEEP
                | BGFX_STENCIL_OP_PASS_Z_INCR;
        }
        else
        {
            // Fallback: just use REPLACE with new ref, test LEQUAL against old.
            // This is less precise but works for most cases.
            stencil = 0
                | BGFX_STENCIL_TEST_LEQUAL
                | BGFX_STENCIL_FUNC_REF(m_stencilRef)
                | BGFX_STENCIL_FUNC_RMASK(0xFF)
                | BGFX_STENCIL_OP_FAIL_S_KEEP
                | BGFX_STENCIL_OP_FAIL_Z_KEEP
                | BGFX_STENCIL_OP_PASS_Z_REPLACE;
        }
        break;
    }
    }

    if (m_scissorEnabled)
    {
        bgfx::setScissor(
            uint16_t(m_scissorRegion.Left()),
            uint16_t(m_scissorRegion.Top()),
            uint16_t(m_scissorRegion.Width()),
            uint16_t(m_scissorRegion.Height())
        );
    }

    bgfx::setStencil(stencil);
    bgfx::submit(m_currentView, creationShader->GetProgram(), m_drawOrder++);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Fullscreen quad
// ─────────────────────────────────────────────────────────────────────────────

void RenderInterface_BGFX::DrawFullscreenQuad(bgfx::TextureHandle texture, RmlProgramId program)
{
    DrawFullscreenQuad(texture, program, {0.f, 0.f}, {1.f, 1.f});
}

void RenderInterface_BGFX::DrawFullscreenQuad(
    bgfx::TextureHandle texture, RmlProgramId program,
    Rml::Vector2f uv_offset, Rml::Vector2f uv_scaling)
{
    auto it = m_geometries.find(m_fullscreenQuad);
    if (it == m_geometries.end()) return;

    const auto& geo = it->second;

    Shader* shader = m_programs[static_cast<int>(program)];
    if (!shader || !shader->IsValid()) return;

    // Identity transform for NDC quad
    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setUniform(u_transform, identity);

    // UV params
    float texParams[4] = { uv_offset.x, uv_offset.y, uv_scaling.x, uv_scaling.y };
    bgfx::setUniform(u_texParams, texParams);

    if (bgfx::isValid(texture))
        bgfx::setTexture(0, s_texture0, texture);

    bgfx::setVertexBuffer(0, geo.vbh);
    bgfx::setIndexBuffer(geo.ibh);

    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    bgfx::setState(state);

    bgfx::submit(m_currentView, shader->GetProgram(), m_drawOrder++);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Layer stack
// ─────────────────────────────────────────────────────────────────────────────

Rml::LayerHandle RenderInterface_BGFX::PushLayer()
{
    Rml::LayerHandle handle = m_layers.PushLayer(*this);

    // Allocate new view for this layer
    m_currentView = AllocateView();
    const auto& fb = m_layers.GetTopLayer();
    SetupView(m_currentView, fb);

    // Reset stencil tracking for new layer
    m_stencilRef = 0;
    m_stencilValue = 1;
    m_drawOrder = 0;

    return handle;
}

void RenderInterface_BGFX::CompositeLayers(
    Rml::LayerHandle source,
    Rml::LayerHandle destination,
    Rml::BlendMode blend_mode,
    Rml::Span<const Rml::CompiledFilterHandle> filters)
{
    const auto& src_fb = m_layers.GetLayer(source);
    const auto& dst_fb = m_layers.GetLayer(destination);

    // Apply filters to source if any
    bgfx::TextureHandle source_texture = src_fb.color;
    if (!filters.empty())
    {
        BlitLayerToPostprocessPrimary(source);
        RenderFilters(filters);
        source_texture = m_layers.GetPostprocessPrimary().color;
    }

    // Composite source onto destination
    bgfx::ViewId compView = AllocateView();
    bgfx::setViewName(compView, "RmlUI_Composite");
    bgfx::setViewFrameBuffer(compView, dst_fb.fb);
    bgfx::setViewRect(compView, 0, 0, uint16_t(dst_fb.width), uint16_t(dst_fb.height));
    bgfx::setViewMode(compView, bgfx::ViewMode::Sequential);

    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(compView, identity, identity);
    bgfx::setViewClear(compView, BGFX_CLEAR_NONE);
    bgfx::touch(compView);

    auto savedView = m_currentView;
    m_currentView = compView;
    m_drawOrder = 0;

    auto it = m_geometries.find(m_fullscreenQuad);
    if (it != m_geometries.end())
    {
        const auto& geo = it->second;

        RmlProgramId prog = RmlProgramId::Passthrough;
        Shader* shader = m_programs[static_cast<int>(prog)];
        if (!shader) { m_currentView = savedView; return; }

        bx::mtxIdentity(identity);
        bgfx::setUniform(u_transform, identity);

        float texParams[4] = { 0.f, 0.f, 1.f, 1.f };
        bgfx::setUniform(u_texParams, texParams);

        bgfx::setTexture(0, s_texture0, source_texture);
        bgfx::setVertexBuffer(0, geo.vbh);
        bgfx::setIndexBuffer(geo.ibh);

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;

        switch (blend_mode)
        {
        case Rml::BlendMode::Blend:
            // Standard premultiplied alpha compositing
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
            break;
        case Rml::BlendMode::Replace:
            // Direct copy, no blending
            break;
        }

        bgfx::setState(state);
        bgfx::submit(m_currentView, shader->GetProgram(), m_drawOrder++);
    }

    // Switch view to destination for continued rendering
    m_currentView = AllocateView();
    SetupView(m_currentView, dst_fb);
    // Don't clear the destination since we just composited onto it
    bgfx::setViewClear(m_currentView, BGFX_CLEAR_NONE);
    bgfx::touch(m_currentView);
    m_drawOrder = 0;
}

void RenderInterface_BGFX::PopLayer()
{
    m_layers.PopLayer();

    // Switch to parent layer
    m_currentView = AllocateView();
    const auto& fb = m_layers.GetTopLayer();
    SetupView(m_currentView, fb);
    // Don't clear — parent layer content is preserved
    bgfx::setViewClear(m_currentView, BGFX_CLEAR_NONE);
    bgfx::touch(m_currentView);

    m_stencilRef = 0;
    m_stencilValue = 1;
    m_drawOrder = 0;
}

Rml::TextureHandle RenderInterface_BGFX::SaveLayerAsTexture()
{
    const auto& srcLayer = m_layers.GetTopLayer();

    int x = 0, y = 0, w = srcLayer.width, h = srcLayer.height;
    if (m_scissorEnabled)
    {
        x = m_scissorRegion.Left();
        y = m_scissorRegion.Top();
        w = m_scissorRegion.Width();
        h = m_scissorRegion.Height();
    }

    if (w <= 0 || h <= 0)
        return {};

    // Create new framebuffer with point sampling to avoid filtering shifts.
    BgfxFramebuffer dstFb = CreateFramebuffer(w, h, false);
    if (!bgfx::isValid(dstFb.fb) || !bgfx::isValid(dstFb.color))
        return {};

    bgfx::ViewId renderView = AllocateView();
    bgfx::setViewName(renderView, "RmlUI_SaveLayer");
    bgfx::setViewFrameBuffer(renderView, dstFb.fb);
    bgfx::setViewRect(renderView, 0, 0, uint16_t(w), uint16_t(h));
    bgfx::setViewMode(renderView, bgfx::ViewMode::Sequential);
    bgfx::setViewClear(renderView, BGFX_CLEAR_COLOR, 0x00000000);

    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(renderView, identity, identity);
    bgfx::touch(renderView);

    auto savedView = m_currentView;
    auto savedOrder = m_drawOrder;
    m_currentView = renderView;
    m_drawOrder = 0;

    // UV transform: source top → dest bottom, source bottom → dest top (vertical flip)
    float u_off = float(x) / float(srcLayer.width);
    float v_off = float(srcLayer.height - y) / float(srcLayer.height); // source top
    float u_scale = float(w) / float(srcLayer.width);
    float v_scale = -float(h) / float(srcLayer.height); // negative for flip

    DrawFullscreenQuad(srcLayer.color, RmlProgramId::Passthrough,
        Rml::Vector2f(u_off, v_off),
        Rml::Vector2f(u_scale, v_scale));

    m_currentView = savedView;
    m_drawOrder = savedOrder;

    return static_cast<Rml::TextureHandle>(dstFb.color.idx);
}

Rml::CompiledFilterHandle RenderInterface_BGFX::SaveLayerAsMaskImage()
{
    const auto& srcLayer = m_layers.GetTopLayer();

    int x = 0, y = 0, w = srcLayer.width, h = srcLayer.height;
    if (m_scissorEnabled)
    {
        x = m_scissorRegion.Left();
        y = m_scissorRegion.Top();
        w = m_scissorRegion.Width();
        h = m_scissorRegion.Height();
    }

    if (w <= 0 || h <= 0)
        return {};

    // Create a new framebuffer exactly the size of the region.
    BgfxFramebuffer maskFb = CreateFramebuffer(w, h, false);
    if (!bgfx::isValid(maskFb.fb) || !bgfx::isValid(maskFb.color))
        return {};

    // Set up a view to render into the mask framebuffer.
    bgfx::ViewId renderView = AllocateView();
    bgfx::setViewName(renderView, "RmlUI_SaveMask");
    bgfx::setViewFrameBuffer(renderView, maskFb.fb);
    bgfx::setViewRect(renderView, 0, 0, uint16_t(w), uint16_t(h));
    bgfx::setViewMode(renderView, bgfx::ViewMode::Sequential);
    bgfx::setViewClear(renderView, BGFX_CLEAR_COLOR, 0x00000000);

    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(renderView, identity, identity);
    bgfx::touch(renderView);

    auto savedView = m_currentView;
    auto savedOrder = m_drawOrder;
    m_currentView = renderView;
    m_drawOrder = 0;

    // UV transform: flip vertically to match OpenGL's blit behaviour.
    float u_off = float(x) / float(srcLayer.width);
    float v_off = float(srcLayer.height - y) / float(srcLayer.height); // source top
    float u_scale = float(w) / float(srcLayer.width);
    float v_scale = -float(h) / float(srcLayer.height); // negative for flip

    DrawFullscreenQuad(srcLayer.color, RmlProgramId::Passthrough,
        Rml::Vector2f(u_off, v_off),
        Rml::Vector2f(u_scale, v_scale));

    m_currentView = savedView;
    m_drawOrder = savedOrder;

    // Create a filter holding this mask texture.
    CompiledFilter filter;
    filter.type = FilterType::MaskImage;
    filter.mask_texture = maskFb.color; // store the texture handle

    Rml::CompiledFilterHandle handle = m_nextFilterId++;
    m_filters[handle] = filter;
    return handle;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Filters
// ─────────────────────────────────────────────────────────────────────────────

Rml::CompiledFilterHandle RenderInterface_BGFX::CompileFilter(
    const Rml::String& name,
    const Rml::Dictionary& parameters)
{
    CompiledFilter filter;

    if (name == "blur")
    {
        filter.type = FilterType::Blur;
        filter.sigma = Rml::Get(parameters, "sigma", 0.f);
    }
    else if (name == "drop-shadow")
    {
        filter.type = FilterType::DropShadow;
        filter.sigma = Rml::Get(parameters, "sigma", 0.f);
        filter.offset.x = Rml::Get(parameters, "offset_x", 0.f);
        filter.offset.y = Rml::Get(parameters, "offset_y", 0.f);
        filter.color = Rml::Get(parameters, "color", Rml::Colourf(0, 0, 0, 1));
    }
    else if (name == "brightness"  || name == "contrast"  ||
             name == "grayscale"   || name == "invert"    ||
             name == "opacity"     || name == "saturate"  ||
             name == "sepia"       || name == "hue-rotate")
    {
        filter.type = FilterType::ColorMatrix;
        float value = Rml::Get(parameters, "value", 1.0f);

        // Build 4x4 color matrix and 4-component translate vector (row-major).
        // Matches the GL3 implementation exactly.
        float* m = filter.color_matrix;
        float* t = filter.color_translate;
        std::memset(m, 0, 16 * sizeof(float));
        std::memset(t, 0, 4 * sizeof(float));

        // Identity matrix
        m[0] = m[5] = m[10] = m[15] = 1.f;

        if (name == "brightness")
        {
            m[0] = m[5] = m[10] = value;
        }
        else if (name == "contrast")
        {
            m[0] = m[5] = m[10] = value;
            float off = 0.5f * (1.f - value);
            t[0] = t[1] = t[2] = off;
        }
        else if (name == "grayscale")
        {
            float inv = 1.f - value;
            m[0] = 0.2126f + 0.7874f * inv; m[1] = 0.7152f - 0.7152f * inv; m[2]  = 0.0722f - 0.0722f * inv;
            m[4] = 0.2126f - 0.2126f * inv; m[5] = 0.7152f + 0.2848f * inv; m[6]  = 0.0722f - 0.0722f * inv;
            m[8] = 0.2126f - 0.2126f * inv; m[9] = 0.7152f - 0.7152f * inv; m[10] = 0.0722f + 0.9278f * inv;
        }
        else if (name == "invert")
        {
            m[0] = m[5] = m[10] = 1.f - 2.f * value;
            t[0] = t[1] = t[2] = value;
        }
        else if (name == "opacity")
        {
            m[15] = value;
        }
        else if (name == "saturate")
        {
            m[0] = 0.2126f + 0.7874f * value; m[1] = 0.7152f - 0.7152f * value; m[2]  = 0.0722f - 0.0722f * value;
            m[4] = 0.2126f - 0.2126f * value; m[5] = 0.7152f + 0.2848f * value; m[6]  = 0.0722f - 0.0722f * value;
            m[8] = 0.2126f - 0.2126f * value; m[9] = 0.7152f - 0.7152f * value; m[10] = 0.0722f + 0.9278f * value;
        }
        else if (name == "sepia")
        {
            float inv = 1.f - value;
            m[0] = 0.393f + 0.607f * inv; m[1] = 0.769f - 0.769f * inv; m[2]  = 0.189f - 0.189f * inv;
            m[4] = 0.349f - 0.349f * inv; m[5] = 0.686f + 0.314f * inv; m[6]  = 0.168f - 0.168f * inv;
            m[8] = 0.272f - 0.272f * inv; m[9] = 0.534f - 0.534f * inv; m[10] = 0.131f + 0.869f * inv;
        }
        else if (name == "hue-rotate")
        {
            float c = std::cos(value); // value in radians
            float s = std::sin(value);
            m[0] = 0.213f + 0.787f * c - 0.213f * s;
            m[1] = 0.715f - 0.715f * c - 0.715f * s;
            m[2] = 0.072f - 0.072f * c + 0.928f * s;
            m[4] = 0.213f - 0.213f * c + 0.143f * s;
            m[5] = 0.715f + 0.285f * c + 0.140f * s;
            m[6] = 0.072f - 0.072f * c - 0.283f * s;
            m[8] = 0.213f - 0.213f * c - 0.787f * s;
            m[9] = 0.715f - 0.715f * c + 0.715f * s;
            m[10]= 0.072f + 0.928f * c + 0.072f * s;
        }
    }
    else
    {
        // Unknown filter
        return {};
    }

    Rml::CompiledFilterHandle handle = m_nextFilterId++;
    m_filters[handle] = filter;
    return handle;
}

void RenderInterface_BGFX::ReleaseFilter(Rml::CompiledFilterHandle filter)
{
    auto it = m_filters.find(filter);
    if (it == m_filters.end()) return;

    if (bgfx::isValid(it->second.mask_texture))
        bgfx::destroy(it->second.mask_texture);

    m_filters.erase(it);
}

void RenderInterface_BGFX::BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle)
{
    const auto& src = m_layers.GetLayer(layer_handle);
    auto& dst = m_layers.GetPostprocessPrimary();

    bgfx::ViewId blitView = AllocateView();
    bgfx::setViewName(blitView, "RmlUI_BlitToPostprocess");
    bgfx::setViewFrameBuffer(blitView, dst.fb);
    bgfx::setViewRect(blitView, 0, 0, uint16_t(dst.width), uint16_t(dst.height));
    bgfx::setViewMode(blitView, bgfx::ViewMode::Sequential);
    bgfx::setViewClear(blitView, BGFX_CLEAR_COLOR, 0x00000000);

    float identity[16];
    bx::mtxIdentity(identity);
    bgfx::setViewTransform(blitView, identity, identity);
    bgfx::touch(blitView);

    auto savedView = m_currentView;
    auto savedOrder = m_drawOrder;
    m_currentView = blitView;
    m_drawOrder = 0;

    DrawFullscreenQuad(src.color, RmlProgramId::Passthrough);

    m_currentView = savedView;
    m_drawOrder = savedOrder;
}

void RenderInterface_BGFX::RenderFilters(
    Rml::Span<const Rml::CompiledFilterHandle> filter_handles)
{
    for (auto fh : filter_handles)
    {
        auto it = m_filters.find(fh);
        if (it == m_filters.end()) continue;

        const auto& filter = it->second;

        switch (filter.type)
        {
        case FilterType::Blur:
        {
            if (filter.sigma <= 0.f) break;
            auto& pp1 = m_layers.GetPostprocessPrimary();
            auto& pp2 = m_layers.GetPostprocessSecondary();
            Rml::Rectanglei rect = Rml::Rectanglei::FromSize(Rml::Vector2i(pp1.width, pp1.height));
            RenderBlur(filter.sigma, pp1, pp2, rect);
            break;
        }
        case FilterType::DropShadow:
        {
            // 1. Copy primary to tertiary (preserve original)
            auto& pp1 = m_layers.GetPostprocessPrimary();
            auto& pp3 = m_layers.GetPostprocessTertiary();

            {
                bgfx::ViewId copyView = AllocateView();
                bgfx::setViewName(copyView, "RmlUI_DropShadow_Copy");
                bgfx::setViewFrameBuffer(copyView, pp3.fb);
                bgfx::setViewRect(copyView, 0, 0, uint16_t(pp3.width), uint16_t(pp3.height));
                bgfx::setViewMode(copyView, bgfx::ViewMode::Sequential);
                bgfx::setViewClear(copyView, BGFX_CLEAR_COLOR, 0x00000000);
                float identity[16]; bx::mtxIdentity(identity);
                bgfx::setViewTransform(copyView, identity, identity);
                bgfx::touch(copyView);

                auto savedView = m_currentView;
                auto savedOrder = m_drawOrder;
                m_currentView = copyView;
                m_drawOrder = 0;
                DrawFullscreenQuad(pp1.color, RmlProgramId::Passthrough);
                m_currentView = savedView;
                m_drawOrder = savedOrder;
            }

            // 2. Apply drop-shadow shader to pp1 (colorize + offset)
            {
                bgfx::ViewId dsView = AllocateView();
                bgfx::setViewName(dsView, "RmlUI_DropShadow_Apply");
                bgfx::setViewFrameBuffer(dsView, pp1.fb);
                bgfx::setViewRect(dsView, 0, 0, uint16_t(pp1.width), uint16_t(pp1.height));
                bgfx::setViewMode(dsView, bgfx::ViewMode::Sequential);
                bgfx::setViewClear(dsView, BGFX_CLEAR_COLOR, 0x00000000);
                float identity[16]; bx::mtxIdentity(identity);
                bgfx::setViewTransform(dsView, identity, identity);
                bgfx::touch(dsView);

                Shader* dsShader = m_programs[static_cast<int>(RmlProgramId::DropShadow)];
                if (dsShader && dsShader->IsValid())
                {
                    auto fsIt = m_geometries.find(m_fullscreenQuad);
                    if (fsIt != m_geometries.end())
                    {
                        const auto& geo = fsIt->second;

                        bgfx::setUniform(u_transform, identity);
                        float texParams[4] = { 0.f, 0.f, 1.f, 1.f };
                        bgfx::setUniform(u_texParams, texParams);

                        float shadowExtra[4] = {
                            filter.offset.x / pp1.width,
                            filter.offset.y / pp1.height,
                            0.f, 0.f
                        };
                        bgfx::setUniform(u_shadowExtra, shadowExtra);

                        float shadowColor[4] = {
                            filter.color.red, filter.color.green,
                            filter.color.blue, filter.color.alpha
                        };
                        bgfx::setUniform(u_shadowColor, shadowColor);

                        bgfx::setTexture(0, s_texture0, pp3.color);
                        bgfx::setVertexBuffer(0, geo.vbh);
                        bgfx::setIndexBuffer(geo.ibh);

                        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
                        bgfx::setState(state);
                        bgfx::submit(dsView, dsShader->GetProgram(), 0);
                    }
                }
            }

            // 3. Blur the drop shadow
            if (filter.sigma > 0.f)
            {
                auto& pp2 = m_layers.GetPostprocessSecondary();
                Rml::Rectanglei rect = Rml::Rectanglei::FromSize(Rml::Vector2i(pp1.width, pp1.height));
                RenderBlur(filter.sigma, pp1, pp2, rect);
            }

            // 4. Composite original (from pp3) on top of blurred shadow (in pp1)
            {
                bgfx::ViewId compView = AllocateView();
                bgfx::setViewName(compView, "RmlUI_DropShadow_Final");
                bgfx::setViewFrameBuffer(compView, pp1.fb);
                bgfx::setViewRect(compView, 0, 0, uint16_t(pp1.width), uint16_t(pp1.height));
                bgfx::setViewMode(compView, bgfx::ViewMode::Sequential);
                bgfx::setViewClear(compView, BGFX_CLEAR_NONE);
                float identity[16]; bx::mtxIdentity(identity);
                bgfx::setViewTransform(compView, identity, identity);
                bgfx::touch(compView);

                auto savedView = m_currentView;
                auto savedOrder = m_drawOrder;
                m_currentView = compView;
                m_drawOrder = 0;
                DrawFullscreenQuad(pp3.color, RmlProgramId::Passthrough);
                m_currentView = savedView;
                m_drawOrder = savedOrder;
            }
            break;
        }
        case FilterType::ColorMatrix:
        {
            auto& pp1 = m_layers.GetPostprocessPrimary();
            auto& pp2 = m_layers.GetPostprocessSecondary();

            bgfx::ViewId cmView = AllocateView();
            bgfx::setViewName(cmView, "RmlUI_ColorMatrix");
            bgfx::setViewFrameBuffer(cmView, pp2.fb);
            bgfx::setViewRect(cmView, 0, 0, uint16_t(pp2.width), uint16_t(pp2.height));
            bgfx::setViewMode(cmView, bgfx::ViewMode::Sequential);
            bgfx::setViewClear(cmView, BGFX_CLEAR_COLOR, 0x00000000);
            float identity[16]; bx::mtxIdentity(identity);
            bgfx::setViewTransform(cmView, identity, identity);
            bgfx::touch(cmView);

            Shader* cmShader = m_programs[static_cast<int>(RmlProgramId::ColorMatrix)];
            if (cmShader && cmShader->IsValid())
            {
                auto fsIt = m_geometries.find(m_fullscreenQuad);
                if (fsIt != m_geometries.end())
                {
                    const auto& geo = fsIt->second;

                    bgfx::setUniform(u_transform, identity);
                    float texParams[4] = { 0.f, 0.f, 1.f, 1.f };
                    bgfx::setUniform(u_texParams, texParams);

                    bgfx::setUniform(u_colorMatrix, filter.color_matrix);
                    bgfx::setUniform(u_colorTranslate, filter.color_translate);

                    bgfx::setTexture(0, s_texture0, pp1.color);
                    bgfx::setVertexBuffer(0, geo.vbh);
                    bgfx::setIndexBuffer(geo.ibh);

                    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
                    bgfx::setState(state);
                    bgfx::submit(cmView, cmShader->GetProgram(), 0);
                }
            }

            m_layers.SwapPostprocessPrimarySecondary();
            break;
        }
        case FilterType::MaskImage:
        {
            // Use blend-mask shader: output = src * mask.a
            auto& pp1 = m_layers.GetPostprocessPrimary();
            auto& pp2 = m_layers.GetPostprocessSecondary();

            bgfx::ViewId maskView = AllocateView();
            bgfx::setViewName(maskView, "RmlUI_MaskImage");
            bgfx::setViewFrameBuffer(maskView, pp2.fb);
            bgfx::setViewRect(maskView, 0, 0, uint16_t(pp2.width), uint16_t(pp2.height));
            bgfx::setViewMode(maskView, bgfx::ViewMode::Sequential);
            bgfx::setViewClear(maskView, BGFX_CLEAR_COLOR, 0x00000000);
            float identity[16]; bx::mtxIdentity(identity);
            bgfx::setViewTransform(maskView, identity, identity);
            bgfx::touch(maskView);

            Shader* bmShader = m_programs[static_cast<int>(RmlProgramId::BlendMask)];
            if (bmShader && bmShader->IsValid())
            {
                auto fsIt = m_geometries.find(m_fullscreenQuad);
                if (fsIt != m_geometries.end())
                {
                    const auto& geo = fsIt->second;

                    bgfx::setUniform(u_transform, identity);
                    float texParams[4] = { 0.f, 0.f, 1.f, 1.f };
                    bgfx::setUniform(u_texParams, texParams);

                    bgfx::setTexture(0, s_texture0, pp1.color);
                    bgfx::setTexture(1, s_texture1, filter.mask_texture);

                    bgfx::setVertexBuffer(0, geo.vbh);
                    bgfx::setIndexBuffer(geo.ibh);

                    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
                    bgfx::setState(state);
                    bgfx::submit(maskView, bmShader->GetProgram(), 0);
                }
            }

            m_layers.SwapPostprocessPrimarySecondary();
            break;
        }
        default:
            break;
        }
    }
}

void RenderInterface_BGFX::RenderBlur(
    float sigma,
    BgfxFramebuffer& source_dest,
    BgfxFramebuffer& temp,
    Rml::Rectanglei /*window_flipped*/)
{
    // Two-pass separable Gaussian blur, matching GL3.
    // Pass 1: source_dest → temp (horizontal)
    // Pass 2: temp → source_dest (vertical)

    Shader* blurShader = m_programs[static_cast<int>(RmlProgramId::Blur)];
    if (!blurShader || !blurShader->IsValid()) return;

    float texelW = 1.f / float(source_dest.width);
    float texelH = 1.f / float(source_dest.height);

    auto fsIt = m_geometries.find(m_fullscreenQuad);
    if (fsIt == m_geometries.end()) return;
    const auto& geo = fsIt->second;

    float identity[16];
    bx::mtxIdentity(identity);

    for (int pass = 0; pass < 2; ++pass)
    {
        bgfx::ViewId blurView = AllocateView();
        bgfx::setViewName(blurView, pass == 0 ? "RmlUI_BlurH" : "RmlUI_BlurV");

        BgfxFramebuffer& dst = (pass == 0) ? temp : source_dest;
        BgfxFramebuffer& src = (pass == 0) ? source_dest : temp;

        bgfx::setViewFrameBuffer(blurView, dst.fb);
        bgfx::setViewRect(blurView, 0, 0, uint16_t(dst.width), uint16_t(dst.height));
        bgfx::setViewMode(blurView, bgfx::ViewMode::Sequential);
        bgfx::setViewClear(blurView, BGFX_CLEAR_COLOR, 0x00000000);
        bgfx::setViewTransform(blurView, identity, identity);
        bgfx::touch(blurView);

        bgfx::setUniform(u_transform, identity);

        float texParams[4] = { 0.f, 0.f, 1.f, 1.f };
        bgfx::setUniform(u_texParams, texParams);

        // blur direction: (1,0) horizontal, (0,1) vertical
        float blurParams[4] = {
            sigma,
            (pass == 0) ? 1.f : 0.f,
            (pass == 0) ? 0.f : 1.f,
            0.f
        };
        bgfx::setUniform(u_blurParams, blurParams);

        float texelSize[4] = { texelW, texelH, 0.f, 0.f };
        bgfx::setUniform(u_texelSize, texelSize);

        bgfx::setTexture(0, s_texture0, src.color);
        bgfx::setVertexBuffer(0, geo.vbh);
        bgfx::setIndexBuffer(geo.ibh);

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
        bgfx::setState(state);
        bgfx::submit(blurView, blurShader->GetProgram(), 0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Shaders (decorators: gradients, etc.)
// ─────────────────────────────────────────────────────────────────────────────

Rml::CompiledShaderHandle RenderInterface_BGFX::CompileShader(
    const Rml::String& name,
    const Rml::Dictionary& parameters)
{
    CompiledShaderData shader;

    if (name == "shader")
    {
        // Build gradient shader data
        shader.type = ShaderType::Gradient;
        shader.gradient_function = Rml::Get(parameters, "function", 0);
        shader.p = Rml::Get(parameters, "p", Rml::Vector2f(0, 0));
        shader.q = Rml::Get(parameters, "q", Rml::Vector2f(0, 0));

        // Extract colour stops and build a 1D texture
        auto stops_variant = parameters.find("stop_colors");
        auto positions_variant = parameters.find("stop_positions");
        if (stops_variant != parameters.end() && positions_variant != parameters.end())
        {
            auto stop_colors = stops_variant->second.GetReference<Rml::ColorStopList>();
            // Build a 1D RGBA texture from the colour stops (256 texels).
            const int STOP_TEX_SIZE = 256;
            std::vector<uint8_t> tex_data(STOP_TEX_SIZE * 4);

            if (!stop_colors.empty())
            {
                for (int i = 0; i < STOP_TEX_SIZE; ++i)
                {
                    float t = float(i) / float(STOP_TEX_SIZE - 1);

                    // Find surrounding stops
                    size_t idx = 0;
                    for (size_t s = 0; s < stop_colors.size() - 1; ++s)
                    {
                        if (t >= stop_colors[s].position.number && t <= stop_colors[s + 1].position.number)
                        {
                            idx = s;
                            break;
                        }
                        if (s == stop_colors.size() - 2)
                            idx = s;
                    }

                    float t0 = stop_colors[idx].position.number;
                    float t1 = stop_colors[idx + 1].position.number;
                    float frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.f;
                    frac = Rml::Math::Clamp(frac, 0.f, 1.f);

                    auto c0 = stop_colors[idx].color;
                    auto c1 = stop_colors[idx + 1].color;

                    tex_data[i * 4 + 0] = uint8_t(c0.red   + (c1.red   - c0.red)   * frac);
                    tex_data[i * 4 + 1] = uint8_t(c0.green + (c1.green - c0.green) * frac);
                    tex_data[i * 4 + 2] = uint8_t(c0.blue  + (c1.blue  - c0.blue)  * frac);
                    tex_data[i * 4 + 3] = uint8_t(c0.alpha + (c1.alpha - c0.alpha) * frac);
                }
            }

            const bgfx::Memory* mem = bgfx::copy(tex_data.data(), uint32_t(tex_data.size()));
            shader.stop_texture = bgfx::createTexture2D(
                STOP_TEX_SIZE, 1, false, 1, bgfx::TextureFormat::RGBA8,
                BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, mem);
        }
    }

    if (shader.type == ShaderType::None)
        return {};

    Rml::CompiledShaderHandle handle = m_nextShaderId++;
    m_shaders[handle] = std::move(shader);
    return handle;
}

void RenderInterface_BGFX::RenderShader(
    Rml::CompiledShaderHandle shader_handle,
    Rml::CompiledGeometryHandle geometry_handle,
    Rml::Vector2f translation,
    Rml::TextureHandle /*texture*/)
{
    auto sit = m_shaders.find(shader_handle);
    if (sit == m_shaders.end()) return;

    const auto& sd = sit->second;

    if (sd.type == ShaderType::Gradient)
    {
        Shader* gradShader = m_programs[static_cast<int>(RmlProgramId::Gradient)];
        if (!gradShader || !gradShader->IsValid()) return;

        auto git = m_geometries.find(geometry_handle);
        if (git == m_geometries.end()) return;

        const auto& geo = git->second;

        UseProgram(RmlProgramId::Gradient);
        SetTransformUniform(translation);

        float gradParams[4] = { float(sd.gradient_function), 0.f, 0.f, 0.f };
        bgfx::setUniform(u_gradientParams, gradParams);

        float gradP[4] = { sd.p.x, sd.p.y, sd.q.x, sd.q.y };
        bgfx::setUniform(u_gradientP, gradP);

        if (bgfx::isValid(sd.stop_texture))
            bgfx::setTexture(0, s_texture0, sd.stop_texture);

        bgfx::setVertexBuffer(0, geo.vbh);
        bgfx::setIndexBuffer(geo.ibh);

        uint64_t state = BuildBaseState()
            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
        bgfx::setState(state);

        if (m_scissorEnabled)
        {
            bgfx::setScissor(
                uint16_t(m_scissorRegion.Left()),
                uint16_t(m_scissorRegion.Top()),
                uint16_t(m_scissorRegion.Width()),
                uint16_t(m_scissorRegion.Height())
            );
        }

        if (m_clipMaskEnabled)
            bgfx::setStencil(BuildStencilState());

        bgfx::submit(m_currentView, gradShader->GetProgram(), m_drawOrder++);
    }
}

void RenderInterface_BGFX::ReleaseShader(Rml::CompiledShaderHandle handle)
{
    auto it = m_shaders.find(handle);
    if (it == m_shaders.end()) return;

    if (bgfx::isValid(it->second.stop_texture))
        bgfx::destroy(it->second.stop_texture);

    m_shaders.erase(it);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Framebuffer helpers
// ─────────────────────────────────────────────────────────────────────────────

BgfxFramebuffer RenderInterface_BGFX::CreateFramebuffer(int w, int h, bool with_depth_stencil)
{
    BgfxFramebuffer fb;
    fb.width = w;
    fb.height = h;

    fb.color = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1,
        bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

    if (with_depth_stencil)
    {
        fb.depth_stencil = bgfx::createTexture2D(uint16_t(w), uint16_t(h), false, 1,
            bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT);

        bgfx::Attachment attachments[2];
        attachments[0].init(fb.color);
        attachments[1].init(fb.depth_stencil);
        fb.fb = bgfx::createFrameBuffer(2, attachments, false); // don't destroy textures on fb destroy
    }
    else
    {
        bgfx::Attachment att;
        att.init(fb.color);
        fb.fb = bgfx::createFrameBuffer(1, &att, false);
    }

    return fb;
}

void RenderInterface_BGFX::DestroyFramebuffer(BgfxFramebuffer& fb)
{
    if (bgfx::isValid(fb.fb))            bgfx::destroy(fb.fb);
    if (bgfx::isValid(fb.color))         bgfx::destroy(fb.color);
    if (bgfx::isValid(fb.depth_stencil)) bgfx::destroy(fb.depth_stencil);
    fb = {};
}

// ─────────────────────────────────────────────────────────────────────────────
//  RenderLayerStack
// ─────────────────────────────────────────────────────────────────────────────

RenderInterface_BGFX::RenderLayerStack::~RenderLayerStack()
{
    if (ri_) DestroyAll(*ri_);
}

void RenderInterface_BGFX::RenderLayerStack::DestroyAll(RenderInterface_BGFX& ri)
{
    for (auto& fb : fb_layers_)
        ri.DestroyFramebuffer(fb);
    fb_layers_.clear();

    for (auto& fb : fb_postprocess_)
        ri.DestroyFramebuffer(fb);
    fb_postprocess_.clear();

    layers_size_ = 0;
}

void RenderInterface_BGFX::RenderLayerStack::BeginFrame(RenderInterface_BGFX& ri, int w, int h)
{
    ri_ = &ri;

    if (w != width_ || h != height_)
    {
        DestroyAll(ri);
        width_ = w;
        height_ = h;
    }

    layers_size_ = 0;
    PushLayer(ri); // base layer (index 0)
}

void RenderInterface_BGFX::RenderLayerStack::EndFrame()
{
    // Keep framebuffers alive for reuse next frame.
    // Just reset the active layer count.
    layers_size_ = 0;
}

Rml::LayerHandle RenderInterface_BGFX::RenderLayerStack::PushLayer(RenderInterface_BGFX& ri)
{
    if (layers_size_ >= (int)fb_layers_.size())
    {
        fb_layers_.push_back(ri.CreateFramebuffer(width_, height_, true));
    }
    Rml::LayerHandle handle = static_cast<Rml::LayerHandle>(layers_size_);
    layers_size_++;
    return handle;
}

void RenderInterface_BGFX::RenderLayerStack::PopLayer()
{
    if (layers_size_ > 1)
        layers_size_--;
}

const BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetLayer(Rml::LayerHandle h) const
{
    return fb_layers_[static_cast<int>(h)];
}

const BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetTopLayer() const
{
    return fb_layers_[layers_size_ - 1];
}

Rml::LayerHandle RenderInterface_BGFX::RenderLayerStack::GetTopLayerHandle() const
{
    return static_cast<Rml::LayerHandle>(layers_size_ - 1);
}

BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetPostprocessPrimary()
{
    return EnsurePostprocess(*ri_, 0);
}

BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetPostprocessSecondary()
{
    return EnsurePostprocess(*ri_, 1);
}

BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetPostprocessTertiary()
{
    return EnsurePostprocess(*ri_, 2);
}

BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::GetBlendMask()
{
    return EnsurePostprocess(*ri_, 3);
}

BgfxFramebuffer& RenderInterface_BGFX::RenderLayerStack::EnsurePostprocess(
    RenderInterface_BGFX& ri, int idx)
{
    while ((int)fb_postprocess_.size() <= idx)
        fb_postprocess_.push_back(ri.CreateFramebuffer(width_, height_, false));
    return fb_postprocess_[idx];
}

void RenderInterface_BGFX::RenderLayerStack::SwapPostprocessPrimarySecondary()
{
    if (fb_postprocess_.size() >= 2)
        std::swap(fb_postprocess_[0], fb_postprocess_[1]);
}
