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

// ---------------------------------------------------------------------------
// Texture handle encoding
//
// BGFX texture handles start at idx=0 for the first valid texture, while
// Rml::TextureHandle uses 0 as the sentinel for "no texture". To avoid the
// collision we store (bgfx_idx + 1) as the RmlUi handle and undo the offset
// whenever we need the real BGFX handle back.
//
//   BGFX idx 0  -> RmlUi handle 1   (first real texture)
//   BGFX idx 1  -> RmlUi handle 2   ...
//   RmlUi handle 0               -> "no texture" (never produced by us)
// ---------------------------------------------------------------------------
static inline Rml::TextureHandle  ToRmlHandle(bgfx::TextureHandle th) { return (Rml::TextureHandle)(th.idx + 1u); }
static inline bgfx::TextureHandle ToBgfxHandle(Rml::TextureHandle  rml) { return bgfx::TextureHandle{ (uint16_t)(rml - 1u) }; }

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Frame lifecycle
// ---------------------------------------------------------------------------

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

    // Clear stencil to 0 at the start of each frame. Colour is NOT cleared
    // here; call Clear() explicitly if you want that. The stencil clear is
    // mandatory so m_stencilRef can restart from 0 safely.
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);

    // BGFX's default mode reorders draws by state for throughput, which
    // breaks the painter's-algorithm compositing that RmlUi relies on.
    bgfx::setViewMode(m_uiView, bgfx::ViewMode::Sequential);

    // Reset per-frame clip state.
    m_stencilRef = 0;
    m_clipMaskEnabled = false;

    // Mirror GL3 BeginFrame(): reset the CSS transform so the cached base
    // matrix is always valid, regardless of what happened last frame.
    SetTransform(nullptr);
}

void RenderInterface_BGFX::EndFrame() {}

void RenderInterface_BGFX::Clear()
{
    // Called after BeginFrame(), so m_uiView is already valid.
    // Keep the stencil clear that BeginFrame() scheduled.
    bgfx::setViewClear(m_uiView, BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

Rml::CompiledGeometryHandle RenderInterface_BGFX::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int>         indices)
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

    // bgfx::copy() makes an internal copy that BGFX owns, so the local vector
    // can be destroyed safely at end of scope.
    g->vb = bgfx::createVertexBuffer(
        bgfx::copy(rmlVerts.data(), (uint32_t)(rmlVerts.size() * sizeof(RmlUiVertex))),
        m_vertexLayout);

    g->ib = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(), (uint32_t)(indices.size() * sizeof(int))),
        BGFX_BUFFER_INDEX32);

    g->numIndices = (uint32_t)indices.size();
    return (Rml::CompiledGeometryHandle)g;
}

void RenderInterface_BGFX::RenderGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f               translation,
    Rml::TextureHandle          tex)
{
    if (!handle) return;
    auto* g = (CompiledGeometry*)handle;

    // tex == 0 means "colour-only" (the sentinel, never a real BGFX idx=0
    // texture because we offset all real handles by +1 via ToRmlHandle).
    Shader* shader = (tex == 0) ? m_shaderColor : m_shaderTexture;
    shader->UseProgram();

    if (tex != 0)
        shader->SetTexture("s_tex", ToBgfxHandle(tex));

    shader->SetUniform("_transform", BuildFullTransform(translation));

    if (m_scissorEnabled)
        bgfx::setScissor(m_scissor.Left(), m_scissor.Top(), m_scissor.Width(), m_scissor.Height());

    bgfx::setVertexBuffer(0, g->vb);
    bgfx::setIndexBuffer(g->ib);

    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
    BgfxStateManager::Apply();

    // When a clip mask is active, only pass fragments whose stencil value
    // equals the current reference. This mirrors GL3's GL_EQUAL stencil test.
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

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

Rml::TextureHandle RenderInterface_BGFX::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source)
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

    // Premultiply alpha so it composes correctly with the premultiplied blend.
    for (size_t i = 0; i < (size_t)w * h * 4; i += 4)
    {
        uint8_t a = pixels[i + 3];
        pixels[i + 0] = (uint8_t)((uint16_t)pixels[i + 0] * a / 255);
        pixels[i + 1] = (uint8_t)((uint16_t)pixels[i + 1] * a / 255);
        pixels[i + 2] = (uint8_t)((uint16_t)pixels[i + 2] * a / 255);
    }

    bgfx::TextureHandle th = bgfx::createTexture2D(
        (uint16_t)w, (uint16_t)h, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
        BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC,
        bgfx::copy(pixels, w * h * 4));

    stbi_image_free(pixels);

    ResourceStatistics::Instance().registerResource(ResourceType::Texture, (uintptr_t)th.idx, (size_t)w * h * 4, source);
    ResourceStatistics::Instance().setResourceName(ResourceType::Texture, (uintptr_t)th.idx, source);

    return ToRmlHandle(th);
}

Rml::TextureHandle RenderInterface_BGFX::GenerateTexture(
    Rml::Span<const Rml::byte> source_data,
    Rml::Vector2i              source_dimensions)
{
    bgfx::TextureHandle th = bgfx::createTexture2D(
        (uint16_t)source_dimensions.x, (uint16_t)source_dimensions.y,
        false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        bgfx::copy(source_data.data(), (uint32_t)source_data.size()));

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
    if (transform)
        m_transform = glm::make_mat4(transform->data());
    else
        m_transform = glm::mat4(1.0f);

    m_transformDirty = true;
}

// ---------------------------------------------------------------------------
// Clip mask  (stencil-based)
//
// GL3 uses a mutable global stencil and clears it on each Set/SetInverse
// operation. BGFX does not expose glClear(STENCIL) mid-frame per view, so
// instead we use a monotonically-increasing reference value:
//
//   Set          ->  ++ref, write ref into the clip shape, test == ref
//   SetInverse   ->  ++ref, write ref everywhere, then write (ref-1) into
//                    the clip shape, test == ref (i.e. "not the shape")
//   Intersect    ->  write (ref+1) only where the stencil already == ref,
//                    then bump ref.  Pixels outside the previous clip were
//                    already < ref and stay there, so they fail the new test.
//
// This lets multiple consecutive clip operations compose correctly without
// ever needing to wipe the buffer, and is safe up to 255 Set-class calls
// per frame (more than any realistic UI scene needs).
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
    auto* g = (CompiledGeometry*)geometry;
    if (!g) return;

    glm::mat4 fullTransform = BuildFullTransform(translation);

    switch (operation)
    {
        // -----------------------------------------------------------------
        // Set: write a new ref value into the clip shape.
        // Pixels outside it keep their old value and will fail the test.
        // -----------------------------------------------------------------
    case Op::Set:
    {
        ++m_stencilRef;

        // Write m_stencilRef wherever the geometry covers, regardless of
        // whatever value was there before (GL_ALWAYS pass).
        uint32_t stencilFlags =
            BGFX_STENCIL_TEST_ALWAYS |
            BGFX_STENCIL_FUNC_REF(m_stencilRef) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_REPLACE;

        SubmitStencilWrite(g, fullTransform, stencilFlags);
        break;
    }

    // -----------------------------------------------------------------
    // SetInverse: everything OUTSIDE the shape should pass, so we:
    //   1. Fill the entire screen with the new ref value.
    //   2. Write (ref-1) — the old value — back into the clip shape.
    // Now outside pixels == ref (pass), inside pixels == ref-1 (fail).
    // -----------------------------------------------------------------
    case Op::SetInverse:
    {
        ++m_stencilRef;

        // Step 1: flood the whole viewport with m_stencilRef.
        SubmitFullscreenStencilFill(m_stencilRef);

        // Step 2: punch back (ref-1) into the shape area.
        uint8_t innerValue = m_stencilRef - 1;
        uint32_t stencilFlags =
            BGFX_STENCIL_TEST_ALWAYS |
            BGFX_STENCIL_FUNC_REF(innerValue) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_REPLACE;

        SubmitStencilWrite(g, fullTransform, stencilFlags);
        break;
    }

    // -----------------------------------------------------------------
    // Intersect: keep only pixels that were already inside the previous
    // clip AND are inside this new shape.  We do this by incrementing
    // the stencil value only where the geometry covers AND the current
    // value already equals ref (the previous clip boundary).
    // -----------------------------------------------------------------
    case Op::Intersect:
    {
        // Only increment pixels where stencil == current ref.
        uint32_t stencilFlags =
            BGFX_STENCIL_TEST_EQUAL |
            BGFX_STENCIL_FUNC_REF(m_stencilRef) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_INCR;

        SubmitStencilWrite(g, fullTransform, stencilFlags);
        ++m_stencilRef;  // subsequent draws test against the incremented value
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

glm::mat4 RenderInterface_BGFX::BuildFullTransform(Rml::Vector2f translation)
{
    if (m_transformDirty)
    {
        m_cachedBaseTransform = m_projection * m_transform;
        m_transformDirty = false;
    }

    // Bake the per-draw pixel translation directly into the matrix.
    // Equivalent to:  projection * css_transform * translate(tx, ty, 0) * vertex
    glm::mat4 translateMat = glm::translate(glm::mat4(1.0f),
        glm::vec3(translation.x, translation.y, 0.0f));
    return m_cachedBaseTransform * translateMat;
}

void RenderInterface_BGFX::SubmitStencilWrite(
    CompiledGeometry* g,
    const glm::mat4& fullTransform,
    uint32_t          stencilFlags)
{
    // Use the colour shader — the fragment output is irrelevant since we
    // suppress all colour channel writes via BGFX state bits.
    m_shaderColor->UseProgram();
    m_shaderColor->SetUniform("_transform", fullTransform);

    bgfx::setVertexBuffer(0, g->vb);
    bgfx::setIndexBuffer(g->ib);

    // Bypass BgfxStateManager entirely: it has no SetColorWrite API.
    // Omitting BGFX_STATE_WRITE_R/G/B/A disables all colour output.
    // Depth test = ALWAYS so the stencil dppass op reliably fires.
    // BGFX per-draw state is fully consumed and reset by Submit(),
    // so nothing here can leak into the next RenderGeometry draw call.
    bgfx::setState(BGFX_STATE_DEPTH_TEST_ALWAYS); // no WRITE_R|G|B|A bits

    bgfx::setStencil(stencilFlags, BGFX_STENCIL_NONE);

    m_shaderColor->Submit(m_uiView);
}

void RenderInterface_BGFX::SubmitFullscreenStencilFill(uint8_t stencilValue)
{
    // Build a screen-space quad covering the entire viewport.
    // We submit it as a transient buffer so there is no allocation overhead.
    float w = (float)m_viewportW;
    float h = (float)m_viewportH;

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer  tib;

    if (!bgfx::allocTransientBuffers(&tvb, m_vertexLayout, 4, &tib, 6))
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "RmlUi BGFX: failed to allocate transient buffers for stencil fill");
        return;
    }

    RmlUiVertex* verts = (RmlUiVertex*)tvb.data;
    verts[0] = { 0.0f, 0.0f, 0.0f,  0, 0, 0, 0,  0.0f, 0.0f };
    verts[1] = { w,    0.0f, 0.0f,  0, 0, 0, 0,  1.0f, 0.0f };
    verts[2] = { w,    h,    0.0f,  0, 0, 0, 0,  1.0f, 1.0f };
    verts[3] = { 0.0f, h,    0.0f,  0, 0, 0, 0,  0.0f, 1.0f };

    uint16_t* idx = (uint16_t*)tib.data;
    idx[0] = 0; idx[1] = 1; idx[2] = 2;
    idx[3] = 0; idx[4] = 2; idx[5] = 3;

    // Use the identity projection so the quad already IS in NDC after
    // multiplication, i.e. pass through via the same _transform uniform path.
    glm::mat4 ndcTransform = m_projection;  // already maps viewport -> NDC

    m_shaderColor->UseProgram();
    m_shaderColor->SetUniform("_transform", ndcTransform);

    bgfx::setVertexBuffer(0, &tvb);
    bgfx::setIndexBuffer(&tib);

    // Same pattern as SubmitStencilWrite: no colour-write bits, bypasses
    // BgfxStateManager to avoid the missing SetColorWrite API.
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