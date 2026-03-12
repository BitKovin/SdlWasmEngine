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

    bgfx::VertexLayout m_vertexLayout;

    Shader* m_shaderColor = nullptr;
    Shader* m_shaderTexture = nullptr;

    glm::mat4 m_projection{ 1.0f };
    glm::mat4 m_transform{ 1.0f };

    // Cached (projection * css_transform). Recomputed only when SetTransform()
    // is called. Per-draw translation is multiplied in at draw time to produce
    // the final matrix that goes to the _transform uniform.
    glm::mat4 m_cachedBaseTransform{ 1.0f };
    bool      m_transformDirty = true;

    bool            m_scissorEnabled = false;
    Rml::Rectanglei m_scissor{};

    uint16_t m_viewportW = 0;
    uint16_t m_viewportH = 0;

    bgfx::ViewId m_uiView = 0;
};