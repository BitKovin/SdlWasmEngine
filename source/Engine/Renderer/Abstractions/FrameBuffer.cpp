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
    m_viewId = -1;
}

Framebuffer::~Framebuffer() {
    destroyFrameBuffer();
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

    //bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);

    // Set the view rect to the first color attachment's dimensions, or
    // the depth attachment if no color attachments exist.
    RenderTexture* primary = nullptr;
    for (auto* c : m_colorAttachments) {
        if (c) { primary = c; break; }
    }

}

// -----------------------------------------------------------------------
// Attachment management
// -----------------------------------------------------------------------
void Framebuffer::attachColor(RenderTexture* texture, uint32_t attachmentIndex) {
    if (!texture)
        throw std::invalid_argument("Framebuffer::attachColor: texture is null");

    // Clear any pending cubemap-face state
    m_cubemapFace = -1;
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
    m_cubemapFace = -1;
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

    m_cubemapFace = static_cast<int32_t>(face);
    m_cubemapSource = cubemap;
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
// resolve — copy auto-resolved color into the target FBO.
//
// bgfx::blit on OpenGL maps to glCopyImageSubData, which requires identical
// sample counts and can NEVER copy MSAA → single-sample.  The only GL path
// that resolves MSAA is glBlitFramebuffer, which bgfx invokes internally
// at pass-end when a color attachment carries BGFX_RESOLVE_AUTO_GEN_MIPS
// (set in rebuild()).  bgfx writes the resolved result into a companion
// single-sample texture owned by the FrameBufferHandle.
// bgfx::getTexture(m_frameBuffer, i) returns that companion — a real
// single-sample handle distinct from m_colorAttachments[i]->textureHandle()
// which is still the raw MSAA surface.  Blitting companion(1x)→target(1x)
// is a same-sample copy and glCopyImageSubData accepts it.
// -----------------------------------------------------------------------
void Framebuffer::resolve(Framebuffer& target) {
    for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
        if (!m_colorAttachments[i]) continue;

        RenderTexture* dst = target.colorAttachment(static_cast<uint32_t>(i));

        // Companion single-sample texture produced by bgfx's internal
        // glBlitFramebuffer resolve — NOT the raw MSAA texture handle.
        bgfx::TextureHandle resolvedSrc =
            bgfx::getTexture(m_frameBuffer, static_cast<uint8_t>(i));
        if (!bgfx::isValid(resolvedSrc))
            continue;

        bgfx::ViewId blitView = ViewIdManager::GiveNextId();
        bgfx::blit(blitView,
            dst->textureHandle(), 0, 0, 0, 0,
            resolvedSrc, 0, 0, 0, 0,
            static_cast<uint16_t>(dst->width()),
            static_cast<uint16_t>(dst->height()),
            1);
    }

    // Depth is BGFX_TEXTURE_RT_WRITE_ONLY — no resolve path in bgfx.
}

// -----------------------------------------------------------------------
// resolveDepthOnly — no-op.
// MSAA depth is BGFX_TEXTURE_RT_WRITE_ONLY; bgfx has no resolve path for it.
// -----------------------------------------------------------------------
void Framebuffer::resolveDepthOnly(Framebuffer& /*target*/) {
    // Intentional no-op.
}

// -----------------------------------------------------------------------
// Bind — configure this framebuffer's view for upcoming draw calls.
// -----------------------------------------------------------------------
void Framebuffer::bind(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h)
{
    if (!bgfx::isValid(m_frameBuffer))
        return; // nothing attached yet

    // Tell the draw-call layer which view to submit to.
    m_viewId = ViewIdManager::GiveNextId();

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