// RenderTexture.cpp  —  bgfx port
#include "RenderTexture.h"

#include <Profiling/ResourceStatistics.hpp>

#include <stdexcept>

#include <string>


#include "ViewIdManager.h"

// -----------------------------------------------------------------------
// Format mapping
// -----------------------------------------------------------------------
bgfx::TextureFormat::Enum RenderTexture::toBgfxFormat(TextureFormat fmt) {
    switch (fmt) {
    case TextureFormat::R8:              return bgfx::TextureFormat::R8;
    case TextureFormat::RG8:             return bgfx::TextureFormat::RG8;
    case TextureFormat::RGB8:            return bgfx::TextureFormat::RGB8;
    case TextureFormat::RGBA8:           return bgfx::TextureFormat::RGBA8;
    case TextureFormat::R16F:            return bgfx::TextureFormat::R16F;
    case TextureFormat::RG16F:           return bgfx::TextureFormat::RG16F;
    case TextureFormat::RGBA16F:         return bgfx::TextureFormat::RGBA16F;
    case TextureFormat::R32F:            return bgfx::TextureFormat::R32F;
    case TextureFormat::RG32F:           return bgfx::TextureFormat::RG32F;
    case TextureFormat::RGBA32F:         return bgfx::TextureFormat::RGBA32F;
    case TextureFormat::Depth16:         return bgfx::TextureFormat::D16;
    case TextureFormat::Depth24:         return bgfx::TextureFormat::D24;
    case TextureFormat::Depth32F:        return bgfx::TextureFormat::D32F;
    case TextureFormat::Depth24Stencil8: return bgfx::TextureFormat::D24S8;
    }
    throw std::runtime_error("RenderTexture: unknown TextureFormat");
}

size_t RenderTexture::bytesPerPixel(TextureFormat fmt) {
    switch (fmt) {
    case TextureFormat::R8:               return 1;
    case TextureFormat::RG8:              return 2;
    case TextureFormat::RGB8:             return 3;
    case TextureFormat::RGBA8:            return 4;
    case TextureFormat::R16F:             return 2;
    case TextureFormat::RG16F:            return 4;
    case TextureFormat::RGB16F:           return 6;
    case TextureFormat::RGBA16F:          return 8;
    case TextureFormat::R32F:             return 4;
    case TextureFormat::RG32F:            return 8;
    case TextureFormat::RGB32F:           return 12;
    case TextureFormat::RGBA32F:          return 16;
    case TextureFormat::Depth16:          return 2;
    case TextureFormat::Depth24:          return 3;
    case TextureFormat::Depth32F:         return 4;
    case TextureFormat::Depth24Stencil8:  return 4;
    case TextureFormat::Depth32FStencil8: return 5;
    }
    return 4;
}

uint64_t RenderTexture::msaaFlag(uint32_t samples) {
    switch (samples) {
    case 2:  return BGFX_TEXTURE_RT_MSAA_X2;
    case 4:  return BGFX_TEXTURE_RT_MSAA_X4;
    case 8:  return BGFX_TEXTURE_RT_MSAA_X8;
    case 16: return BGFX_TEXTURE_RT_MSAA_X16;
    default: return BGFX_TEXTURE_RT;            // single-sample RT
    }
}

// -----------------------------------------------------------------------
// Constructor / destructor
// -----------------------------------------------------------------------
RenderTexture::RenderTexture(uint32_t width, uint32_t height,
    TextureFormat format,
    TextureType   type,
    bool          sampleDepth,
    uint64_t      samplerFlags,
    uint32_t      samples)
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_type(type)
    , m_samples(samples)
    , m_samplerFlags(samplerFlags)
    , m_sampleDepth(sampleDepth)
{
    createResources();
}

RenderTexture::~RenderTexture()
{
    destroyResources();
}

// -----------------------------------------------------------------------
// createResources / destroyResources
// -----------------------------------------------------------------------
void RenderTexture::createResources() {
    bgfx::TextureFormat::Enum bgfxFmt = toBgfxFormat(m_format);

    // Base RT flags + MSAA
    uint64_t rtFlag = msaaFlag(m_samples);

    // Depth-compare sampler
    uint64_t samplerExtra = m_sampleDepth ? BGFX_SAMPLER_COMPARE_LEQUAL : 0;

    // Blit destination flag — always set so copyFrom() works on any RT
    uint64_t blitFlag = BGFX_TEXTURE_BLIT_DST;

    uint64_t flags = rtFlag | blitFlag | m_samplerFlags | samplerExtra;

    if (m_type == TextureType::Cubemap) {
        m_texture = bgfx::createTextureCube(
            (uint16_t)m_width,
            false,
            1,
            bgfxFmt,
            flags
        );

        if (!bgfx::isValid(m_texture))
            throw std::runtime_error("RenderTexture: bgfx failed to create cubemap texture");

        // Default to face 0; call setCubemapFace() to switch.
        bgfx::Attachment att;
        att.init(m_texture, bgfx::Access::Write, 0);
        m_frameBuffer = bgfx::createFrameBuffer(1, &att);
    }
    else {
        m_texture = bgfx::createTexture2D(
            (uint16_t)m_width,
            (uint16_t)m_height,
            false,
            1,
            bgfxFmt,
            flags
        );

        if (!bgfx::isValid(m_texture))
            throw std::runtime_error("RenderTexture: bgfx failed to create texture");

        // Pass false so the framebuffer does NOT own the texture —
        // we destroy them independently in destroyResources().
        m_frameBuffer = bgfx::createFrameBuffer(1, &m_texture, false);
    }

    if (!bgfx::isValid(m_frameBuffer))
        throw std::runtime_error("RenderTexture: bgfx failed to create framebuffer");

    // Resource statistics
    size_t texSize = (size_t)m_width * m_height * bytesPerPixel(m_format) * m_samples;
    if (m_type == TextureType::Cubemap) texSize *= 6;

    ResourceStatistics::Instance().registerResource(
        ResourceType::RenderTexture, m_texture.idx, texSize, m_name);
}

void RenderTexture::destroyResources() {
    ResourceStatistics::Instance().unregisterResource(
        ResourceType::RenderTexture, m_texture.idx);

    if (bgfx::isValid(m_frameBuffer)) {
        bgfx::destroy(m_frameBuffer);
        m_frameBuffer = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(m_texture)) {
        bgfx::destroy(m_texture);
        m_texture = BGFX_INVALID_HANDLE;
    }
}

// -----------------------------------------------------------------------
// Render-target configuration
// -----------------------------------------------------------------------
void RenderTexture::setAsRenderTarget(uint16_t x, uint16_t y,
    uint16_t w, uint16_t h)
{
    m_viewId = ViewIdManager::GiveNextId();
    bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);
    bgfx::setViewRect(m_viewId,
        x, y,
        w ? w : (uint16_t)m_width,
        h ? h : (uint16_t)m_height);
}

void RenderTexture::setCubemapFace(uint8_t face) {
    if (m_type != TextureType::Cubemap)
        return;

    if (bgfx::isValid(m_frameBuffer))
        bgfx::destroy(m_frameBuffer);

    bgfx::Attachment att;
    att.init(m_texture, bgfx::Access::Write, face);
    m_frameBuffer = bgfx::createFrameBuffer(1, &att);

    m_viewId = ViewIdManager::GiveNextId();
    bgfx::setViewFrameBuffer(m_viewId, m_frameBuffer);
}

// -----------------------------------------------------------------------
// Resize
// -----------------------------------------------------------------------
bool RenderTexture::resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return false;

    destroyResources();
    m_width = width;
    m_height = height;
    createResources();

    if (!m_name.empty())
        bgfx::setName(m_texture, m_name.c_str());

    return true;
}

// -----------------------------------------------------------------------
// Copy (blit)
// bgfx::blit is recorded into a view and executes before that view's draws.
// We use the destination RT's own view so ordering is guaranteed.
// -----------------------------------------------------------------------
void RenderTexture::copyFrom(const RenderTexture* src) {
    if (!src)
        throw std::runtime_error("RenderTexture::copyFrom: null source");

    if (m_width != src->width() ||
        m_height != src->height() ||
        m_format != src->format())
    {
        throw std::runtime_error(
            "RenderTexture::copyFrom: source/destination dimension or format mismatch");
    }

    // Sample counts are intentionally allowed to differ —
    // resolving MSAA → single-sample is the primary use-case here.

    // bgfx::blit(viewId, dst, dstMip, dstX, dstY, dstZ,
    //            src, srcMip, srcX, srcY, srcZ, w, h, d)
    bgfx::blit(m_viewId,
        m_texture, 0, 0, 0, 0,
        src->m_texture, 0, 0, 0, 0,
        (uint16_t)m_width, (uint16_t)m_height, 1);
}

// -----------------------------------------------------------------------
// Mutators
// -----------------------------------------------------------------------
void RenderTexture::setSamples(uint32_t samples) {
    if (samples == m_samples) return;
    destroyResources();
    m_samples = samples;
    createResources();
}

void RenderTexture::setTextureType(TextureType newType) {
    if (newType == m_type) return;
    destroyResources();
    m_type = newType;
    createResources();
}

void RenderTexture::SetName(const std::string& name) {
    m_name = name;
    if (bgfx::isValid(m_texture))
        bgfx::setName(m_texture, name.c_str(), (int32_t)name.size());
    ResourceStatistics::Instance().setResourceName(
        ResourceType::RenderTexture, m_texture.idx, name);
}