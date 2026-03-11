#pragma once

#include <bgfx/bgfx.h>
#include <stdexcept>
#include <string>
#include <cstdint>

// -----------------------------------------------------------------------
// Enums — no longer backed by GL constants
// -----------------------------------------------------------------------

enum class TextureFormat {
    // 8-bit unsigned
    R8, RG8, RGB8, RGBA8,
    // 16-bit float
    R16F, RG16F, RGB16F, RGBA16F,
    // 32-bit float
    R32F, RG32F, RGB32F, RGBA32F,
    // Depth / stencil
    Depth16,
    Depth24,          // maps to D24 in bgfx
    Depth32F,
    Depth24Stencil8,
    Depth32FStencil8
};

enum class TextureType {
    Texture2D,
    Cubemap,
    Texture2DMultisample   // resolved via MSAA flags in bgfx
};

// -----------------------------------------------------------------------
// RenderTexture
// Each instance owns:
//   - one bgfx::TextureHandle     (the GPU texture / attachment)
//   - one bgfx::FrameBufferHandle (wraps the texture as a render target,
//                                   BGFX_INVALID_HANDLE for cubemaps when
//                                   no face is selected)
//   - one bgfx::ViewId            (dedicated view for rendering into this RT)
//
// Usage pattern:
//   rt.setAsRenderTarget();          // configures the internal view
//   bgfx::setViewClear(rt.viewId(), BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, ...);
//   // ... submit draw calls with encoder->submit(rt.viewId(), program);
//   rt.bind(0, s_texSampler);        // later, sample the result
// -----------------------------------------------------------------------

class RenderTexture {
public:
    RenderTexture(uint32_t width, uint32_t height,
        TextureFormat format,
        TextureType   type = TextureType::Texture2D,
        bool          sampleDepth = false,
        uint64_t      samplerFlags = BGFX_SAMPLER_U_CLAMP
        | BGFX_SAMPLER_V_CLAMP
        | BGFX_SAMPLER_MIN_POINT
        | BGFX_SAMPLER_MAG_POINT,
        uint32_t      samples = 0);

    ~RenderTexture();

    // Non-copyable, movable
    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;
    RenderTexture(RenderTexture&&) = default;
    RenderTexture& operator=(RenderTexture&&) = default;

    // -----------------------------------------------------------------------
    // Render-target path
    //   Configures the owned view so that subsequent draw calls submitted
    //   to viewId() render into this texture.
    //   rect defaults to the full texture; supply non-zero args to override.
    // -----------------------------------------------------------------------
    void setAsRenderTarget(uint16_t x = 0, uint16_t y = 0,
        uint16_t w = 0, uint16_t h = 0) const;

    // Set the cubemap face that subsequent renders go into.
    // Only meaningful for TextureType::Cubemap.
    void setCubemapFace(uint8_t face);   // face 0-5

    // -----------------------------------------------------------------------
    // Resize — recreates internal resources
    // -----------------------------------------------------------------------
    bool resize(uint32_t width, uint32_t height);

    // -----------------------------------------------------------------------
    // Copy (blit) the contents of src into this texture.
    // Both must have identical dimensions, format, type, and sample count.
    // Destination texture is created with BGFX_TEXTURE_BLIT_DST automatically.
    // -----------------------------------------------------------------------
    void copyFrom(const RenderTexture* src);

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    bgfx::TextureHandle     textureHandle()     const { return m_texture; }
    bgfx::FrameBufferHandle frameBufferHandle() const { return m_frameBuffer; }
    bgfx::ViewId            viewId()            const { return m_viewId; }

    uint32_t      width()   const { return m_width; }
    uint32_t      height()  const { return m_height; }
    TextureFormat format()  const { return m_format; }
    TextureType   type()    const { return m_type; }
    uint32_t      samples() const { return m_samples; }

    // Legacy numeric ID used by ResourceStatistics
    uint16_t id() const { return m_texture.idx; }

    void setSamples(uint32_t samples);
    void setTextureType(TextureType newType);
    void SetName(const std::string& name);

private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------
    void createResources();
    void destroyResources();

    static bgfx::TextureFormat::Enum toBgfxFormat(TextureFormat fmt);
    static size_t                    bytesPerPixel(TextureFormat fmt);
    static uint64_t                  msaaFlag(uint32_t samples);

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------
    uint32_t      m_width;
    uint32_t      m_height;
    TextureFormat m_format;
    TextureType   m_type;
    uint32_t      m_samples;
    uint64_t      m_samplerFlags;
    bool          m_sampleDepth;
    std::string   m_name;

    bgfx::TextureHandle     m_texture{ BGFX_INVALID_HANDLE };
    bgfx::FrameBufferHandle m_frameBuffer{ BGFX_INVALID_HANDLE };
    bgfx::ViewId            m_viewId{ 0 };
};