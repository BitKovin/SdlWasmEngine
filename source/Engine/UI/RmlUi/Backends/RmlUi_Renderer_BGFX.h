#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <bgfx/bgfx.h>
#include "Shader.hpp"
#include <Renderer/Abstractions/ViewIdManager.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertex layout — must match the $input declaration in vs_rmlui.sc:
//   a_position  (3 floats)
//   a_color0    (4 uint8, normalised)
//   a_texcoord0 (2 floats)
struct RmlUiVertex {
    float   x, y, z;
    uint8_t r, g, b, a;
    float   u, v;
};

class RenderInterface_BGFX : public Rml::RenderInterface
{
public:
    RenderInterface_BGFX();
    ~RenderInterface_BGFX();

    // Call whenever the window / render-target size changes.
    void SetViewport(int width, int height);

    // Call at the start / end of each frame.
    void BeginFrame();
    void EndFrame();

    // Optionally clear the colour buffer for this frame.
    void Clear();

    // -----------------------------------------------------------------------
    // Rml::RenderInterface
    // -----------------------------------------------------------------------
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
        Rml::Span<const int>         indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle handle,
        Rml::Vector2f               translation,
        Rml::TextureHandle          texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle handle) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
        const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source_data,
        Rml::Vector2i              source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

    // Stencil-based clip mask (mirrors GL3's EnableClipMask / RenderToClipMask).
    void EnableClipMask(bool enable) override;
    void RenderToClipMask(Rml::ClipMaskOperation      operation,
        Rml::CompiledGeometryHandle geometry,
        Rml::Vector2f               translation) override;

private:
    // -----------------------------------------------------------------------
    // Internal types
    // -----------------------------------------------------------------------
    struct CompiledGeometry
    {
        bgfx::VertexBufferHandle vb = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle  ib = BGFX_INVALID_HANDLE;
        uint32_t numIndices = 0;
    };

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------

    // Upload both uniforms that every draw call needs, mirroring GL3's
    // SubmitTransformUniform(translation):
    //   bgfx::setUniform(_transform, m_transform)     -- projection * css_transform
    //   bgfx::setUniform(_translate, {tx, ty, 0, 0})  -- per-draw offset
    void SubmitTransformUniforms(Rml::Vector2f translation);

    // Submit geometry for stencil-write only (colour writes suppressed via
    // bgfx::setState without BGFX_STATE_WRITE_R/G/B/A).
    // stencilFlags: BGFX_STENCIL_* describing the write operation.
    void SubmitGeometryToStencil(CompiledGeometry* g,
        Rml::Vector2f     translation,
        uint32_t          stencilFlags);

    // Draw a viewport-covering quad and replace every stencil pixel with
    // `stencilValue`.  Used by SetInverse to pre-fill before punching the
    // clip shape back out.
    void FloodStencil(uint8_t stencilValue);

    // -----------------------------------------------------------------------
    // Geometry / shaders
    // -----------------------------------------------------------------------
    bgfx::VertexLayout m_vertexLayout;

    Shader* m_shaderColor = nullptr;
    Shader* m_shaderTexture = nullptr;

    // Uniform handles created in the constructor and destroyed in the
    // destructor.  We call bgfx::setUniform() directly so we are never at the
    // mercy of whatever overloads Shader::SetUniform happens to provide.
    bgfx::UniformHandle m_uniformTransform = BGFX_INVALID_HANDLE; // mat4
    bgfx::UniformHandle m_uniformTranslate = BGFX_INVALID_HANDLE; // vec4 (.xy used)
    bgfx::UniformHandle m_uniformSampler = BGFX_INVALID_HANDLE; // sampler2D s_tex

    // -----------------------------------------------------------------------
    // Transform state  (mirrors GL3 exactly)
    //
    //   m_projection = ortho projection for the current viewport
    //   m_transform  = m_projection * css_transform   (updated by SetTransform)
    //
    // Both uniforms are re-uploaded on every draw because BGFX consumes all
    // per-draw state after each bgfx::submit() — there is no sticky state.
    // -----------------------------------------------------------------------
    glm::mat4 m_projection{ 1.0f };
    glm::mat4 m_transform{ 1.0f };

    // -----------------------------------------------------------------------
    // Scissor
    // -----------------------------------------------------------------------
    bool            m_scissorEnabled = false;
    Rml::Rectanglei m_scissor{};

    // -----------------------------------------------------------------------
    // Viewport / view
    // -----------------------------------------------------------------------
    uint16_t     m_viewportW = 0;
    uint16_t     m_viewportH = 0;
    bgfx::ViewId m_uiView = 0;

    // -----------------------------------------------------------------------
    // Clip mask  (stencil)
    //
    // GL3 uses glClear(STENCIL_BUFFER_BIT) on every Set/SetInverse, which is
    // not possible mid-frame in BGFX per-view.  Instead we maintain a
    // monotonically-increasing per-frame reference counter:
    //
    //   Set        ->  ++ref; REPLACE ref into shape; test == ref
    //   SetInverse ->  ++ref; flood whole screen with ref; REPLACE (ref-1)
    //                  into shape; test == ref  (outside passes, inside fails)
    //   Intersect  ->  INCR only pixels already at ref, then ++ref;
    //                  only the intersection reaches the new ref value
    //
    // Overflows at 255 Set-class operations per frame — never reached in
    // practice for UI rendering.
    // -----------------------------------------------------------------------
    bool    m_clipMaskEnabled = false;
    uint8_t m_stencilRef = 0;
};