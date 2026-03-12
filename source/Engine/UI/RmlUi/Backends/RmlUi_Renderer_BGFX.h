#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <bgfx/bgfx.h>
#include "Shader.hpp"
#include <Renderer/Abstractions/ViewIdManager.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct RmlUiVertex {
    float   x, y, z;       // a_position (z = 0)
    uint8_t r, g, b, a;    // a_color0 (normalised)
    float   u, v;           // a_texcoord0
};

class RenderInterface_BGFX : public Rml::RenderInterface
{
public:
    RenderInterface_BGFX();
    ~RenderInterface_BGFX();

    void SetViewport(int width, int height);

    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle handle) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source_data, Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;
    void SetTransform(const Rml::Matrix4f* transform) override;

    // Clip mask (stencil-based). Requires the active framebuffer to have a stencil attachment.
    void EnableClipMask(bool enable) override;
    void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;

    void BeginFrame();
    void EndFrame();
    void Clear();

private:
    struct CompiledGeometry
    {
        bgfx::VertexBufferHandle vb = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle  ib = BGFX_INVALID_HANDLE;
        uint32_t numIndices = 0;
    };

    // --- Helpers ---

    // Returns the full MVP matrix for a given per-draw translation, refreshing the
    // cached base transform if SetTransform() has been called since the last draw.
    glm::mat4 BuildFullTransform(Rml::Vector2f translation);

    // Submit compiled geometry for stencil writing only (no colour output).
    // Uses bgfx::setState directly (no BgfxStateManager) to suppress colour
    // writes without needing a SetColorWrite API that doesn't exist.
    // stencilFlags: BGFX_STENCIL_* flags describing the write operation.
    // NOTE: the BGFX view's framebuffer MUST have a stencil attachment for
    //       any of this to take effect. If rendering to the default backbuffer,
    //       pass BGFX_RESET_STENCIL when calling bgfx::reset().
    void SubmitStencilWrite(CompiledGeometry* g, const glm::mat4& fullTransform, uint32_t stencilFlags);

    // Flood the entire viewport with stencilValue in the stencil buffer.
    // Used by RenderToClipMask(SetInverse) to "pre-fill" before punching the
    // clip shape back out.
    void SubmitFullscreenStencilFill(uint8_t stencilValue);

    // --- State ---

    bgfx::VertexLayout m_vertexLayout;

    Shader* m_shaderColor = nullptr;
    Shader* m_shaderTexture = nullptr;

    glm::mat4 m_projection{ 1.0f };
    glm::mat4 m_transform{ 1.0f };

    // Cached (projection * css_transform). Recomputed only when SetTransform() fires.
    // Per-draw translation is folded in at draw time.
    glm::mat4 m_cachedBaseTransform{ 1.0f };
    bool      m_transformDirty = true;

    bool            m_scissorEnabled = false;
    Rml::Rectanglei m_scissor{};

    uint16_t m_viewportW = 0;
    uint16_t m_viewportH = 0;

    bgfx::ViewId m_uiView = 0;

    // Clip mask (stencil) state.
    //
    // m_stencilRef is a monotonically-increasing per-frame counter.  Each
    // Set/SetInverse/Intersect call bumps it so the new test value is always
    // unique — avoiding the GL3 approach of wiping the whole stencil buffer.
    // Overflows at 255 Set-class operations per frame, which is never reached
    // in practice for a UI scene.
    bool    m_clipMaskEnabled = false;
    uint8_t m_stencilRef = 0;
};