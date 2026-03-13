#include "RmlUi_Renderer_BGFX.h"
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Profiling.h>
#include <includedLibraries/stb_image.h>
#include "Profiling/ResourceStatistics.hpp"
#include "BgfxStateManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ShaderManager.h>
#include <vector>

// ---------------------------------------------------------------------------
// Texture handle encoding
//
// BGFX texture handles start at idx=0 for the very first texture created.
// Rml::TextureHandle uses 0 as the sentinel value for "no texture".
// These two conventions collide, so we store (bgfx_idx + 1) as the RmlUi
// handle and subtract 1 when we need the real BGFX handle back.
//
//   BGFX idx 0  ->  RmlUi handle 1   (first real texture, e.g. font atlas)
//   BGFX idx 1  ->  RmlUi handle 2   ...
//   RmlUi handle 0               ->  "no texture"  (never produced here)
// ---------------------------------------------------------------------------
static inline Rml::TextureHandle  ToRmlHandle(bgfx::TextureHandle th) { return static_cast<Rml::TextureHandle>(th.idx + 1u); }
static inline bgfx::TextureHandle ToBgfxHandle(Rml::TextureHandle  rml) { return bgfx::TextureHandle{ static_cast<uint16_t>(rml - 1u) }; }

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

RenderInterface_BGFX::RenderInterface_BGFX()
{
    // Vertex layout — order and types must match vs_rmlui.sc's $input:
    //   a_position  vec3 float
    //   a_color0    vec4 uint8 normalised
    //   a_texcoord0 vec2 float
    m_vertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, /*normalised=*/true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_shaderColor = ShaderManager::GetShaderProgram("rmlui/vs_rmlui", "rmlui/fs_rmlui_color");
    m_shaderTexture = ShaderManager::GetShaderProgram("rmlui/vs_rmlui", "rmlui/fs_rmlui_texture");

    if (!m_shaderColor || !m_shaderTexture)
        Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi: BGFX shaders failed to load");

    // Create the uniform handles ourselves so we can call bgfx::setUniform()
    // directly, bypassing Shader::SetUniform whose vec4 overload may not exist.
    m_uniformTransform = bgfx::createUniform("_transform", bgfx::UniformType::Mat4);
    m_uniformTranslate = bgfx::createUniform("_translate", bgfx::UniformType::Vec4);
    m_uniformSampler = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

    if (!bgfx::isValid(m_uniformTransform) ||
        !bgfx::isValid(m_uniformTranslate) ||
        !bgfx::isValid(m_uniformSampler))
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi: Failed to create BGFX uniform handles");
    }
}

RenderInterface_BGFX::~RenderInterface_BGFX()
{
    if (bgfx::isValid(m_uniformTransform)) bgfx::destroy(m_uniformTransform);
    if (bgfx::isValid(m_uniformTranslate)) bgfx::destroy(m_uniformTranslate);
    if (bgfx::isValid(m_uniformSampler))   bgfx::destroy(m_uniformSampler);
}

// ---------------------------------------------------------------------------
// Viewport
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::SetViewport(int width, int height)
{
    m_viewportW = static_cast<uint16_t>(width);
    m_viewportH = static_cast<uint16_t>(height);

    // Build an orthographic projection identical to GL3's ProjectOrtho:
    //   left=0, right=width, bottom=height, top=0
    // This maps (0,0) to the top-left and (width,height) to the bottom-right,
    // which is the convention RmlUi and CSS expect.
    m_projection = glm::ortho(0.0f, static_cast<float>(width),
        static_cast<float>(height), 0.0f,
        -1.0f, 1.0f);

    // Force m_transform to be refreshed with the new projection on the next
    // SetTransform call.  Mirror GL3 by also resetting to the plain projection.
    m_transform = m_projection;
}

// ---------------------------------------------------------------------------
// Frame lifecycle
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::BeginFrame()
{
    m_uiView = ViewIdManager::GiveNextId();

    bgfx::setViewRect(m_uiView, 0, 0, m_viewportW, m_viewportH);
    bgfx::setViewFrameBuffer(m_uiView, BGFX_INVALID_HANDLE);

    // Clear stencil to 0 so m_stencilRef can restart from 0 each frame.
    // Colour is intentionally NOT cleared here; call Clear() if needed.
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);

    // Preserve the exact RmlUi draw-call submission order.  BGFX's default
    // reorders draws by render state for throughput, which breaks the
    // painter's-algorithm compositing that RmlUi relies on.
    bgfx::setViewMode(m_uiView, bgfx::ViewMode::Sequential);

    // Reset per-frame clip state  (mirrors GL3 BeginFrame stencil init).
    m_stencilRef = 0;
    m_clipMaskEnabled = false;

    // Mirror GL3 BeginFrame(): reset the CSS transform so m_transform always
    // starts as the plain projection, independent of previous frame state.
    SetTransform(nullptr);
}

void RenderInterface_BGFX::EndFrame()
{
    // Nothing to do in BGFX — state is not accumulated between frames.
}

void RenderInterface_BGFX::Clear()
{
    // Called after BeginFrame(), so m_uiView is already valid.
    // Preserve the stencil clear that BeginFrame already scheduled.
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL,
        0x00000000, 1.0f, 0);
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

Rml::CompiledGeometryHandle RenderInterface_BGFX::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int>         indices)
{
    if (vertices.empty() || indices.empty())
        return 0;

    auto* g = new CompiledGeometry();

    // Convert from RmlUi's Vertex layout to our tightly-packed RmlUiVertex.
    std::vector<RmlUiVertex> verts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        const Rml::Vertex& v = vertices[i];
        verts[i] = {
            v.position.x, v.position.y, 0.0f,
            v.colour.red, v.colour.green, v.colour.blue, v.colour.alpha,
            v.tex_coord.x, v.tex_coord.y
        };
    }

    // bgfx::copy() makes an internal copy that BGFX owns, so the local vector
    // can safely be destroyed at end of scope.
    g->vb = bgfx::createVertexBuffer(
        bgfx::copy(verts.data(), static_cast<uint32_t>(verts.size() * sizeof(RmlUiVertex))),
        m_vertexLayout);

    g->ib = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(), static_cast<uint32_t>(indices.size() * sizeof(int))),
        BGFX_BUFFER_INDEX32);

    g->numIndices = static_cast<uint32_t>(indices.size());
    return reinterpret_cast<Rml::CompiledGeometryHandle>(g);
}

void RenderInterface_BGFX::RenderGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f               translation,
    Rml::TextureHandle          tex)
{
    if (!handle) return;
    auto* g = reinterpret_cast<CompiledGeometry*>(handle);

    // Choose shader based on whether we have a texture.
    // tex == 0 is RmlUi's sentinel for "no texture" — never a real BGFX idx=0
    // handle because we offset all real handles by +1 in ToRmlHandle().
    if (tex != 0)
    {
        m_shaderTexture->UseProgram();
        bgfx::setTexture(0, m_uniformSampler, ToBgfxHandle(tex));
    }
    else
    {
        m_shaderColor->UseProgram();
    }

    // Upload _transform and _translate on every draw call.
    // BGFX consumes all per-draw state after each bgfx::submit(), so we
    // cannot rely on any value set in a previous draw — everything must be
    // re-submitted every time, unlike OpenGL's sticky uniforms.
    SubmitTransformUniforms(translation);

    if (m_scissorEnabled)
        bgfx::setScissor(m_scissor.Left(), m_scissor.Top(),
            m_scissor.Width(), m_scissor.Height());

    bgfx::setVertexBuffer(0, g->vb);
    bgfx::setIndexBuffer(g->ib);

    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
    BgfxStateManager::Apply();

    // When a clip mask is active, only pass fragments whose stencil value
    // equals m_stencilRef.  This mirrors GL3's GL_EQUAL stencil function.
    if (m_clipMaskEnabled)
    {
        bgfx::setStencil(
            BGFX_STENCIL_TEST_EQUAL |
            BGFX_STENCIL_FUNC_REF(m_stencilRef) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_KEEP,
            BGFX_STENCIL_NONE);
    }

    // tex != 0: submit with texture shader; tex == 0: submit with color shader.
    if (tex != 0)
        m_shaderTexture->Submit(m_uiView);
    else
        m_shaderColor->Submit(m_uiView);
}

void RenderInterface_BGFX::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
    if (!handle) return;
    auto* g = reinterpret_cast<CompiledGeometry*>(handle);
    if (bgfx::isValid(g->vb)) bgfx::destroy(g->vb);
    if (bgfx::isValid(g->ib)) bgfx::destroy(g->ib);
    delete g;
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

Rml::TextureHandle RenderInterface_BGFX::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source)
{
    Rml::FileInterface* fi = Rml::GetFileInterface();
    Rml::FileHandle fh = fi->Open(source);
    if (!fh) return 0;

    fi->Seek(fh, 0, SEEK_END);
    size_t size = fi->Tell(fh);
    fi->Seek(fh, 0, SEEK_SET);

    std::vector<uint8_t> buf(size);
    fi->Read(buf.data(), size, fh);
    fi->Close(fh);

    int w, h, channels;
    stbi_uc* pixels = stbi_load_from_memory(buf.data(), static_cast<int>(size),
        &w, &h, &channels, STBI_rgb_alpha);
    if (!pixels)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi: Failed to load texture '%s'", source.c_str());
        return 0;
    }

    texture_dimensions = { w, h };

    // Premultiply alpha to match the premultiplied-alpha blend mode.
    for (size_t i = 0; i < static_cast<size_t>(w * h) * 4; i += 4)
    {
        const uint8_t a = pixels[i + 3];
        pixels[i + 0] = static_cast<uint8_t>(static_cast<uint16_t>(pixels[i + 0]) * a / 255);
        pixels[i + 1] = static_cast<uint8_t>(static_cast<uint16_t>(pixels[i + 1]) * a / 255);
        pixels[i + 2] = static_cast<uint8_t>(static_cast<uint16_t>(pixels[i + 2]) * a / 255);
    }

    bgfx::TextureHandle th = bgfx::createTexture2D(
        static_cast<uint16_t>(w), static_cast<uint16_t>(h),
        /*hasMips=*/false, /*numLayers=*/1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
        BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC,
        bgfx::copy(pixels, w * h * 4));

    stbi_image_free(pixels);

    ResourceStatistics::Instance().registerResource(ResourceType::Texture,
        static_cast<uintptr_t>(th.idx),
        static_cast<size_t>(w * h * 4),
        source);
    ResourceStatistics::Instance().setResourceName(ResourceType::Texture,
        static_cast<uintptr_t>(th.idx), source);

    return ToRmlHandle(th);
}

Rml::TextureHandle RenderInterface_BGFX::GenerateTexture(
    Rml::Span<const Rml::byte> source_data,
    Rml::Vector2i              source_dimensions)
{
    bgfx::TextureHandle th = bgfx::createTexture2D(
        static_cast<uint16_t>(source_dimensions.x),
        static_cast<uint16_t>(source_dimensions.y),
        false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        bgfx::copy(source_data.data(), static_cast<uint32_t>(source_data.size())));

    return ToRmlHandle(th);
}

void RenderInterface_BGFX::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    bgfx::TextureHandle th = ToBgfxHandle(texture_handle);
    if (bgfx::isValid(th))
        bgfx::destroy(th);
}

// ---------------------------------------------------------------------------
// Scissor
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::EnableScissorRegion(bool enable)
{
    m_scissorEnabled = enable;
}

void RenderInterface_BGFX::SetScissorRegion(Rml::Rectanglei region)
{
    m_scissor = region;
}

// ---------------------------------------------------------------------------
// Transform
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::SetTransform(const Rml::Matrix4f* transform)
{
    // Mirror GL3 exactly:
    //   this->transform = new_transform ? (projection * (*new_transform)) : projection;
    if (transform)
        m_transform = m_projection * glm::make_mat4(transform->data());
    else
        m_transform = m_projection;
}

// ---------------------------------------------------------------------------
// Clip mask
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::EnableClipMask(bool enable)
{
    m_clipMaskEnabled = enable;
}

void RenderInterface_BGFX::RenderToClipMask(
    Rml::ClipMaskOperation      operation,
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f               translation)
{
    using Op = Rml::ClipMaskOperation;
    auto* g = reinterpret_cast<CompiledGeometry*>(geometry);
    if (!g) return;

    switch (operation)
    {
        // -----------------------------------------------------------------------
        // Set: write a new reference value into the clip shape.
        // Pixels outside it keep their old (lower) value and fail the test.
        // -----------------------------------------------------------------------
    case Op::Set:
    {
        ++m_stencilRef;

        // ALWAYS pass, then REPLACE the stencil pixel with m_stencilRef.
        uint32_t flags =
            BGFX_STENCIL_TEST_ALWAYS |
            BGFX_STENCIL_FUNC_REF(m_stencilRef) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_REPLACE;

        SubmitGeometryToStencil(g, translation, flags);
        break;
    }

    // -----------------------------------------------------------------------
    // SetInverse: everything OUTSIDE the shape should pass.
    // GL3 achieves this by:
    //   1. glClearStencil(1); glClear(STENCIL); glClearStencil(0);
    //   2. REPLACE 0 into the clip shape
    //   3. Test: EQUAL 1 (outside passes, inside fails)
    //
    // BGFX equivalent (no mid-frame stencil clear):
    //   1. ++ref; FloodStencil(ref)          -- fill whole screen with ref
    //   2. REPLACE (ref-1) into clip shape   -- punch inside back down
    //   3. Test: EQUAL ref (outside passes, inside has ref-1 and fails)
    // -----------------------------------------------------------------------
    case Op::SetInverse:
    {
        ++m_stencilRef;

        // Step 1: flood the entire viewport with m_stencilRef.
        FloodStencil(m_stencilRef);

        // Step 2: write (ref-1) back into the clip shape area.
        const uint8_t innerValue = m_stencilRef - 1u;
        uint32_t flags =
            BGFX_STENCIL_TEST_ALWAYS |
            BGFX_STENCIL_FUNC_REF(innerValue) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_REPLACE;

        SubmitGeometryToStencil(g, translation, flags);
        break;
    }

    // -----------------------------------------------------------------------
    // Intersect: keep only pixels inside BOTH the previous clip and this
    // new shape.
    // GL3: INCR where stencil == current_ref, then test == ref+1.
    // BGFX: same logic — INCR only pixels where stencil == m_stencilRef,
    //       then bump the ref so subsequent draws test the new value.
    // -----------------------------------------------------------------------
    case Op::Intersect:
    {
        // Only increment where the stencil already equals the current ref.
        uint32_t flags =
            BGFX_STENCIL_TEST_EQUAL |
            BGFX_STENCIL_FUNC_REF(m_stencilRef) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_INCR;

        SubmitGeometryToStencil(g, translation, flags);
        ++m_stencilRef;
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void RenderInterface_BGFX::SubmitTransformUniforms(Rml::Vector2f translation)
{
    // _transform: projection * css_transform (mat4, column-major GLM layout).
    bgfx::setUniform(m_uniformTransform, glm::value_ptr(m_transform));

    // _translate: per-draw pixel offset.
    // Declared as vec4 in the shader because BGFX does not support vec2/vec3
    // uniforms; only .xy are read in the vertex shader.
    const float t[4] = { translation.x, translation.y, 0.0f, 0.0f };
    bgfx::setUniform(m_uniformTranslate, t);
}

void RenderInterface_BGFX::SubmitGeometryToStencil(
    CompiledGeometry* g,
    Rml::Vector2f     translation,
    uint32_t          stencilFlags)
{
    // We need the correct spatial transform so the stencil mask lands at the
    // right position.  Use the colour shader (fragment output is discarded).
    m_shaderColor->UseProgram();
    SubmitTransformUniforms(translation);

    bgfx::setVertexBuffer(0, g->vb);
    bgfx::setIndexBuffer(g->ib);

    // Bypass BgfxStateManager: omit all BGFX_STATE_WRITE_* bits to suppress
    // colour output.  BGFX per-draw state is fully consumed and reset after
    // each bgfx::submit(), so this cannot leak into the next RenderGeometry.
    bgfx::setState(BGFX_STATE_DEPTH_TEST_ALWAYS);  // no WRITE_R|G|B|A

    bgfx::setStencil(stencilFlags, BGFX_STENCIL_NONE);

    m_shaderColor->Submit(m_uiView);
}

void RenderInterface_BGFX::FloodStencil(uint8_t stencilValue)
{
    // Draw a fullscreen quad in NDC space with an identity transform so every
    // stencil pixel in the viewport gets overwritten with stencilValue.
    // Using NDC coordinates directly avoids any dependency on m_projection or
    // m_viewportW/H, and is correct regardless of BGFX backend.

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer  tib;

    if (!bgfx::allocTransientBuffers(&tvb, m_vertexLayout, 4, &tib, 6))
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "RmlUi: Out of transient buffer space in FloodStencil");
        return;
    }

    // NDC corners: (-1,-1) bottom-left to (1,1) top-right.
    auto* v = reinterpret_cast<RmlUiVertex*>(tvb.data);
    v[0] = { -1.0f, -1.0f, 0.0f,  0, 0, 0, 0,  0.0f, 0.0f };
    v[1] = { 1.0f, -1.0f, 0.0f,  0, 0, 0, 0,  1.0f, 0.0f };
    v[2] = { 1.0f,  1.0f, 0.0f,  0, 0, 0, 0,  1.0f, 1.0f };
    v[3] = { -1.0f,  1.0f, 0.0f,  0, 0, 0, 0,  0.0f, 1.0f };

    auto* idx = reinterpret_cast<uint16_t*>(tib.data);
    idx[0] = 0; idx[1] = 1; idx[2] = 2;
    idx[3] = 0; idx[4] = 2; idx[5] = 3;

    // Identity transform + zero translate: NDC coords pass through unchanged.
    const glm::mat4 identity(1.0f);
    const float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    bgfx::setUniform(m_uniformTransform, glm::value_ptr(identity));
    bgfx::setUniform(m_uniformTranslate, zero);

    m_shaderColor->UseProgram();
    bgfx::setVertexBuffer(0, &tvb);
    bgfx::setIndexBuffer(&tib);

    // No colour writes; only stencil matters.
    bgfx::setState(BGFX_STATE_DEPTH_TEST_ALWAYS);

    bgfx::setStencil(
        BGFX_STENCIL_TEST_ALWAYS |
        BGFX_STENCIL_FUNC_REF(stencilValue) |
        BGFX_STENCIL_FUNC_RMASK(0xFF) |
        BGFX_STENCIL_OP_FAIL_S_REPLACE |
        BGFX_STENCIL_OP_FAIL_Z_REPLACE |
        BGFX_STENCIL_OP_PASS_Z_REPLACE,
        BGFX_STENCIL_NONE);

    m_shaderColor->Submit(m_uiView);
}