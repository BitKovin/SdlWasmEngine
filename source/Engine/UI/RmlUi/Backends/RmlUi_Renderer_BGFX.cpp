#include "RmlUi_Renderer_BGFX.h"
#include <RmlUi/Core/Log.h>
#include <includedLibraries/stb_image.h>
#include "Profiling/ResourceStatistics.hpp"
#include "BgfxStateManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Profiling.h>
#include <vector>
#include <ShaderManager.h>

RenderInterface_BGFX::RenderInterface_BGFX()
{
    m_vertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_shaderColor = ShaderManager::GetShaderProgram("rmlui/vs_rmlui", "rmlui/fs_rmlui_color");
    m_shaderTexture = ShaderManager::GetShaderProgram("rmlui/vs_rmlui", "rmlui/fs_rmlui_texture");

    if (!m_shaderColor || !m_shaderTexture)
        Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi bgfx shaders failed to load");
}

RenderInterface_BGFX::~RenderInterface_BGFX() = default;

void RenderInterface_BGFX::SetViewport(int width, int height)
{
    m_viewportW = (uint16_t)width;
    m_viewportH = (uint16_t)height;
    m_projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    m_transformDirty = true;
}

void RenderInterface_BGFX::BeginFrame()
{
    m_uiView = ViewIdManager::GiveNextId();
    bgfx::setViewRect(m_uiView, 0, 0, m_viewportW, m_viewportH);
    bgfx::setViewFrameBuffer(m_uiView, BGFX_INVALID_HANDLE);
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_NONE);

    // Preserve exact RmlUi draw-call submission order.
    // BGFX's default mode reorders draws by state for throughput, which
    // breaks the painter's-algorithm compositing that RmlUi relies on.
    bgfx::setViewMode(m_uiView, bgfx::ViewMode::Sequential);
}

void RenderInterface_BGFX::EndFrame() {}

void RenderInterface_BGFX::Clear()
{
    // Called after BeginFrame() so m_uiView is valid for this frame.
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_COLOR, 0x00000000);
}

Rml::CompiledGeometryHandle RenderInterface_BGFX::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    if (vertices.empty() || indices.empty()) return 0;

    auto* g = new CompiledGeometry();

    std::vector<RmlUiVertex> rmlVerts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        const auto& v = vertices[i];
        rmlVerts[i] = {
            v.position.x, v.position.y, 0.0f,
            v.colour.red, v.colour.green, v.colour.blue, v.colour.alpha,
            v.tex_coord.x, v.tex_coord.y
        };
    }

    // bgfx::copy() makes an internal copy BGFX owns.
    // bgfx::makeRef() only stores a raw pointer — rmlVerts would be destroyed
    // at end of scope, leaving BGFX pointing at freed memory.
    g->vb = bgfx::createVertexBuffer(
        bgfx::copy(rmlVerts.data(), (uint32_t)(rmlVerts.size() * sizeof(RmlUiVertex))),
        m_vertexLayout);

    g->ib = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(), (uint32_t)(indices.size() * sizeof(int))),
        BGFX_BUFFER_INDEX32);

    g->numIndices = (uint32_t)indices.size();
    return (Rml::CompiledGeometryHandle)g;
}

void RenderInterface_BGFX::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle tex)
{
    if (!handle) return;
    auto* g = (CompiledGeometry*)handle;

    Shader* shader = (tex == 0) ? m_shaderColor : m_shaderTexture;
    shader->UseProgram();

    if (tex != 0)
    {
        // Sampler name must match SAMPLER2D(s_tex, 0) in fs_rmlui_texture.sc exactly.
        shader->SetTexture("s_tex", (bgfx::TextureHandle)(uintptr_t)tex);
    }

    // Rebuild the base (projection * css_transform) only when SetTransform() has
    // been called since the last draw.
    if (m_transformDirty)
    {
        m_cachedBaseTransform = m_projection * m_transform;
        m_transformDirty = false;
    }

    // FIX: bake the per-draw translation directly into the matrix instead of
    // passing it as a separate _translate vec4 uniform.
    //
    // Root cause: Shader::SetUniform has no vec4 overload, so the call
    //   shader->SetUniform("_translate", glm::vec4(...))
    // was silently discarded. The _translate uniform stayed (0,0,0,0), so every
    // element rendered at document position (0,0) — the top-left corner.
    //
    // By building a translation matrix and right-multiplying it here we get
    // exactly the same result:
    //   _transform * vertex = projection * css_transform * translate(tx,ty) * vertex
    //                       = projection * css_transform * (vertex + offset)
    // but with only a single mat4 uniform that is known to work.
    glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, 0.0f));
    glm::mat4 fullTransform = m_cachedBaseTransform * translateMat;

    // BGFX uniforms are consumed per Submit() — they must be re-uploaded every draw.
    shader->SetUniform("_transform", fullTransform);

    if (m_scissorEnabled)
        bgfx::setScissor(m_scissor.Left(), m_scissor.Top(), m_scissor.Width(), m_scissor.Height());

    bgfx::setVertexBuffer(0, g->vb);
    bgfx::setIndexBuffer(g->ib);

    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
    BgfxStateManager::Apply();

    shader->Submit(m_uiView);
}

void RenderInterface_BGFX::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
    if (!handle) return;
    auto* g = (CompiledGeometry*)handle;
    if (bgfx::isValid(g->vb)) bgfx::destroy(g->vb);
    if (bgfx::isValid(g->ib)) bgfx::destroy(g->ib);
    delete g;
}

Rml::TextureHandle RenderInterface_BGFX::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    Rml::FileInterface* file_interface = Rml::GetFileInterface();
    Rml::FileHandle fh = file_interface->Open(source);
    if (!fh) return 0;

    file_interface->Seek(fh, 0, SEEK_END);
    size_t size = file_interface->Tell(fh);
    file_interface->Seek(fh, 0, SEEK_SET);

    std::vector<uint8_t> buffer(size);
    file_interface->Read(buffer.data(), size, fh);
    file_interface->Close(fh);

    int w, h, channels;
    stbi_uc* pixels = stbi_load_from_memory(buffer.data(), (int)size, &w, &h, &channels, STBI_rgb_alpha);
    if (!pixels)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to load texture %s", source.c_str());
        return 0;
    }

    texture_dimensions = { w, h };

    // Premultiply alpha
    for (size_t i = 0; i < (size_t)w * h * 4; i += 4)
    {
        uint8_t a = pixels[i + 3];
        pixels[i + 0] = (uint8_t)((uint16_t)pixels[i + 0] * a / 255);
        pixels[i + 1] = (uint8_t)((uint16_t)pixels[i + 1] * a / 255);
        pixels[i + 2] = (uint8_t)((uint16_t)pixels[i + 2] * a / 255);
    }

    bgfx::TextureHandle th = bgfx::createTexture2D(
        (uint16_t)w, (uint16_t)h, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC,
        bgfx::copy(pixels, w * h * 4));

    stbi_image_free(pixels);

    ResourceStatistics::Instance().registerResource(ResourceType::Texture, (uintptr_t)th.idx, (size_t)w * h * 4, source);
    ResourceStatistics::Instance().setResourceName(ResourceType::Texture, (uintptr_t)th.idx, source);

    return (Rml::TextureHandle)th.idx;
}

Rml::TextureHandle RenderInterface_BGFX::GenerateTexture(Rml::Span<const Rml::byte> source_data, Rml::Vector2i source_dimensions)
{
    bgfx::TextureHandle th = bgfx::createTexture2D(
        (uint16_t)source_dimensions.x, (uint16_t)source_dimensions.y, false, 1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        bgfx::copy(source_data.data(), (uint32_t)source_data.size()));

    return (Rml::TextureHandle)th.idx;
}

void RenderInterface_BGFX::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    bgfx::TextureHandle th{ (uint16_t)(uintptr_t)texture_handle };
    if (bgfx::isValid(th))
        bgfx::destroy(th);
}

void RenderInterface_BGFX::EnableScissorRegion(bool enable) { m_scissorEnabled = enable; }
void RenderInterface_BGFX::SetScissorRegion(Rml::Rectanglei region) { m_scissor = region; }

void RenderInterface_BGFX::SetTransform(const Rml::Matrix4f* transform)
{
    if (transform)
        m_transform = glm::make_mat4(transform->data());
    else
        m_transform = glm::mat4(1.0f);

    m_transformDirty = true;
}