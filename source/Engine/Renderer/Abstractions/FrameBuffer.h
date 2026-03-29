#pragma once
#include "RenderTexture.h"
#include <bgfx/bgfx.h>
#include <vector>
#include <stdexcept>
#include <cstdint>

// -----------------------------------------------------------------------
// Framebuffer — bgfx port
//
// Each Framebuffer owns:
//   - one bgfx::FrameBufferHandle  (composite of all attached textures)
//   - one bgfx::ViewId             (dedicated view; matches RenderTexture
//                                   convention so callers always render to
//                                   a named view)
//
// Usage pattern:
//   fb.attachColor(colorRT, 0);
//   fb.attachDepth(depthRT);
//   bgfx::setViewClear(fb.viewId(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, ...);
//   bgfx::setViewRect (fb.viewId(), 0, 0, w, h);
//   encoder->submit(fb.viewId(), program);
//
// To resolve MSAA, use resolve() for a full blit, or resolveDepthOnly()
// after a depth-only pre-pass before color has been written.
// -----------------------------------------------------------------------

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // Non-copyable
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // -----------------------------------------------------------------------
    // Attachment management
    // All attach* calls rebuild the underlying bgfx framebuffer.
    //
    // Recommended attachment order: attachColor() before attachDepth().
    // This ensures the first rebuild() has a valid color attachment from
    // which to derive the view rect, avoiding a transient depth-only phase.
    // -----------------------------------------------------------------------
    void attachColor(RenderTexture* texture, uint32_t attachmentIndex = 0);
    void attachDepth(RenderTexture* texture);

    // Attach a single cubemap face (face 0-5).
    // Pass isDepth=true to attach as the depth target instead of color[0].
    void attachCubemapFace(RenderTexture* cubemap, uint32_t face,
        bool isDepth = false);

    // -----------------------------------------------------------------------
    // Resolve MSAA color+depth → single-sample target.
    // Internally delegates to RenderTexture::copyFrom() (bgfx::blit).
    //
    // IMPORTANT: Only call resolve() after ALL attachments have been written
    // for the current pass. If you only want to copy depth (e.g. after a
    // depth pre-pass before any color writes), call resolveDepthOnly()
    // instead to avoid blitting uninitialized color data.
    // -----------------------------------------------------------------------
    void resolve(Framebuffer& target);

    // Resolve only the depth attachment.
    // Safe to call after a depth-only pre-pass when color is not yet written.
    void resolveDepthOnly(Framebuffer& target);

    // -----------------------------------------------------------------------
    // Bind helpers — configure the owned bgfx view.
    // Call before submitting draw calls for this framebuffer.
    // -----------------------------------------------------------------------
    void bind(uint16_t x = 0, uint16_t y = 0,
        uint16_t w = 0, uint16_t h = 0);

    // No-op in bgfx (views are statically assigned); provided for API parity.
    static void unbind() {}

    void bindDepthOnly(uint16_t x = 0, uint16_t y = 0, uint16_t w = 0, uint16_t h = 0);

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    RenderTexture* colorAttachment(uint32_t index = 0) const;
    RenderTexture* depthAttachment() const { return m_depthAttachment; }

    bgfx::FrameBufferHandle frameBufferHandle() const { return m_frameBuffer; }

private:
    // Destroy and recreate m_frameBuffer from current attachment list.
    void rebuild();
    void destroyFrameBuffer();

    bgfx::ViewId            m_viewId{ 0 };
    bgfx::FrameBufferHandle m_frameBuffer{ BGFX_INVALID_HANDLE };

    std::vector<RenderTexture*> m_colorAttachments;
    RenderTexture* m_depthAttachment = nullptr;

    // Cubemap-face attachment state (face index, isDepth flag).
    // Non-negative when the last attachment was a cubemap face.
    int32_t  m_cubemapFace = -1;
    bool     m_cubemapIsDepth = false;
    RenderTexture* m_cubemapSource = nullptr;
};
