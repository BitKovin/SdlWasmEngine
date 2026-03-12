// FrameBuffer.cpp — bgfx port
#include "FrameBuffer.h"
#include "RenderTexture.h"
#include "ViewIdManager.h"
#include <stdexcept>
#include <algorithm>

// -----------------------------------------------------------------------
// Constructor / destructor
// -----------------------------------------------------------------------
Framebuffer::Framebuffer() {
    m_viewId = ViewIdManager::allocateViewId();
}

Framebuffer::~Framebuffer() {
    destroyFrameBuffer();
    ViewIdManager::deallocateViewId(m_viewId);
}

// -----------------------------------------------------------------------
// Internal helpers
// -----------------------------------------------------------------------
void Framebuffer::destroyFrameBuffer() {
    if (bgfx::isValid(m_frameBuffer)) {
        bgfx::destroy(m_frameBuffer);
        m_frameBuffer = BGFX_INVALID_HANDLE;
    }
}

void Framebuffer::rebuild() {
    destroyFrameBuffer();

    // ---- Cubemap-face path ----
    if (m_cubemapSource && m_cubemapFace >= 0) {
        bgfx::Attachment att;
        att.init(m_cubemapSource->textureHandle(),
            bgfx::Access::Write,
            static_cast<uint16_t>(m_cubemapFace));
        m_frameBuffer = bgfx::createFrameBuffer(1, &att);

        if (!bgfx::isValid(m_frameBuffer))
            throw std::runtime_error(
                "Framebuffer: bgfx failed to create cubemap-face framebuffer");

        bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);

        // Set the view rect from the cubemap source dimensions.
        bgfx::setViewRect(m_viewId, 0, 0,
            static_cast<uint16_t>(m_cubemapSource->width()),
            static_cast<uint16_t>(m_cubemapSource->height()));
        return;
    }

    // ---- General path: collect all attachments ----
    // Color attachments first, depth last — this is the order bgfx expects.
    std::vector<bgfx::Attachment> attachments;

    for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
        if (!m_colorAttachments[i]) continue;
        bgfx::Attachment att;
        att.init(m_colorAttachments[i]->textureHandle(),
            bgfx::Access::Write,
            0 /*layer*/,
            1 /*numMips*/,
            0 /*mip*/,
            BGFX_RESOLVE_AUTO_GEN_MIPS);
        attachments.push_back(att);
    }

    if (m_depthAttachment) {
        bgfx::Attachment att;
        att.init(m_depthAttachment->textureHandle(),
            bgfx::Access::Write,
            0, 1, 0,
            BGFX_RESOLVE_AUTO_GEN_MIPS);
        attachments.push_back(att);
    }

    if (attachments.empty())
        return; // nothing attached yet; defer creation

    m_frameBuffer = bgfx::createFrameBuffer(
        static_cast<uint8_t>(attachments.size()),
        attachments.data()
    );

    if (!bgfx::isValid(m_frameBuffer))
        throw std::runtime_error(
            "Framebuffer: bgfx failed to create framebuffer");

    bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);

    // Set the view rect to the first color attachment's dimensions, or
    // the depth attachment if no color attachments exist.
    RenderTexture* primary = nullptr;
    for (auto* c : m_colorAttachments) {
        if (c) { primary = c; break; }
    }
    if (!primary) primary = m_depthAttachment;
    if (primary) {
        bgfx::setViewRect(m_viewId, 0, 0,
            static_cast<uint16_t>(primary->width()),
            static_cast<uint16_t>(primary->height()));
    }
}

// -----------------------------------------------------------------------
// Attachment management
// -----------------------------------------------------------------------
void Framebuffer::attachColor(RenderTexture* texture, uint32_t attachmentIndex) {
    if (!texture)
        throw std::invalid_argument("Framebuffer::attachColor: texture is null");

    // Clear any pending cubemap-face state
    m_cubemapFace   = -1;
    m_cubemapSource = nullptr;
    m_cubemapIsDepth = false;

    if (attachmentIndex >= m_colorAttachments.size())
        m_colorAttachments.resize(attachmentIndex + 1, nullptr);

    m_colorAttachments[attachmentIndex] = texture;
    rebuild();
}

void Framebuffer::attachDepth(RenderTexture* texture) {
    if (!texture)
        throw std::invalid_argument("Framebuffer::attachDepth: texture is null");

    // Clear any pending cubemap-face state
    m_cubemapFace   = -1;
    m_cubemapSource = nullptr;
    m_cubemapIsDepth = false;

    m_depthAttachment = texture;
    rebuild();
}

void Framebuffer::attachCubemapFace(RenderTexture* cubemap,
    uint32_t        face,
    bool            isDepth)
{
    if (!cubemap)
        throw std::invalid_argument("Framebuffer::attachCubemapFace: cubemap is null");
    if (cubemap->type() != TextureType::Cubemap)
        throw std::invalid_argument("Framebuffer::attachCubemapFace: texture is not a cubemap");
    if (face > 5)
        throw std::out_of_range("Framebuffer::attachCubemapFace: face index must be 0-5");

    m_cubemapFace    = static_cast<int32_t>(face);
    m_cubemapSource  = cubemap;
    m_cubemapIsDepth = isDepth;

    if (isDepth)
        m_depthAttachment = cubemap;
    else {
        if (m_colorAttachments.empty())
            m_colorAttachments.resize(1, nullptr);
        m_colorAttachments[0] = cubemap;
    }

    rebuild();
}

// -----------------------------------------------------------------------
// Resolve MSAA → single-sample
//
// FIX #3 (partial): Callers must NOT invoke resolve() for color attachments
// before color has been written. The depth-only resolve after the pre-pass
// is now done explicitly in RenderCameraForward rather than calling
// resolve() on the full FBO.
//
// bgfx has no explicit "blit framebuffer" call; we delegate to
// RenderTexture::copyFrom() which issues bgfx::blit on the destination
// texture's view (so ordering relative to this framebuffer's view is
// automatically correct as long as viewIds are sequenced properly).
// -----------------------------------------------------------------------
void Framebuffer::resolve(Framebuffer& target) {
    // Resolve color attachments
    for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
        if (!m_colorAttachments[i]) continue;

        RenderTexture* dst = target.colorAttachment(static_cast<uint32_t>(i));
        dst->copyFrom(m_colorAttachments[i]);
    }

    // Resolve depth (if both sides have one)
    if (m_depthAttachment && target.m_depthAttachment)
        target.m_depthAttachment->copyFrom(m_depthAttachment);
}

// -----------------------------------------------------------------------
// resolveDepthOnly — resolve only the depth attachment.
// Use this after a depth-pre-pass before color has been written to avoid
// blitting uninitialized color data into the resolve target.
// -----------------------------------------------------------------------
void Framebuffer::resolveDepthOnly(Framebuffer& target) {
    if (m_depthAttachment && target.m_depthAttachment)
        target.m_depthAttachment->copyFrom(m_depthAttachment);
}

// -----------------------------------------------------------------------
// Bind — configure this framebuffer's view for upcoming draw calls.
// -----------------------------------------------------------------------
void Framebuffer::bind(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h) const
{
    if (!bgfx::isValid(m_frameBuffer))
        return; // nothing attached yet

    // Tell the draw-call layer which view to submit to.
    ViewIdManager::setCurrentViewId(m_viewId);

    bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);

    // Determine effective dimensions
    uint16_t rw = w, rh = h;
    if (rw == 0 || rh == 0) {
        RenderTexture* primary = nullptr;
        for (auto* c : m_colorAttachments) {
            if (c) { primary = c; break; }
        }
        if (!primary) primary = m_depthAttachment;
        if (primary) {
            rw = static_cast<uint16_t>(primary->width());
            rh = static_cast<uint16_t>(primary->height());
        }
    }
    bgfx::setViewRect(m_viewId, x, y, rw, rh);
}

// -----------------------------------------------------------------------
// Accessors
// -----------------------------------------------------------------------
RenderTexture* Framebuffer::colorAttachment(uint32_t index) const {
    if (index >= m_colorAttachments.size() || !m_colorAttachments[index])
        throw std::out_of_range("Framebuffer: no color attachment at index "
            + std::to_string(index));
    return m_colorAttachments[index];
}
