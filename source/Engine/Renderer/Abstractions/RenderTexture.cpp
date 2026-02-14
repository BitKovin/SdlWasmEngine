// RenderTexture.cpp
#include "RenderTexture.h"
#include "../../gl.h"
#include <stdexcept>

#include <Profiling/ResourceStatistics.hpp>

//------------------------------------------------------------------------------
// On WebGL (Emscripten) we must fall back to single‐sample 2D only
#if defined(GL_ES_PROFILE)
#define DISABLE_MULTISAMPLE
#endif

RenderTexture::RenderTexture(uint32_t width, uint32_t height,
    TextureFormat format,
    TextureType type, bool sampleDepth,
    GLenum minFilter,
    GLenum magFilter,
    GLenum wrap,
    uint32_t samples)
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_type(type)
    , m_minFilter(minFilter), m_magFilter(magFilter), m_wrapF(wrap)
    , m_samples(samples), m_sampleDepth(sampleDepth)
{
#ifndef DISABLE_MULTISAMPLE
    validateSampleCount();
#else
    // force single‐sample on WebGL
    m_samples = 1;
    // if someone passed Texture2DMultisample, silently treat as Texture2D
    if (m_type == TextureType::Texture2DMultisample)
        m_type = TextureType::Texture2D;
#endif

    glGenTextures(1, &m_id);
    allocateStorage();
    setParameters(minFilter, magFilter, wrap);

    // Set up attachment type and FBO if applicable
    auto [internalFmt, baseFmt, dataType] = getFormatInfo();
    if (baseFmt == GL_DEPTH_STENCIL) {
        m_attachment = GL_DEPTH_STENCIL_ATTACHMENT;
    }
    else if (baseFmt == GL_DEPTH_COMPONENT) {
        m_attachment = GL_DEPTH_ATTACHMENT;
    }
    else {
        m_attachment = GL_COLOR_ATTACHMENT0;
    }
    setupFramebuffer();
}

RenderTexture::~RenderTexture() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_id);

	ResourceStatistics::Instance().unregisterResource(ResourceType::RenderTexture, m_id);

}

void RenderTexture::bind(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(static_cast<GLenum>(m_type), m_id);
}

void RenderTexture::unbind() const {
    glBindTexture(static_cast<GLenum>(m_type), 0);
}

bool RenderTexture::resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return false;
    m_width = width;
    m_height = height;
    allocateStorage();
    return true;
}

void RenderTexture::setTextureType(TextureType newType)
{
    if (newType == m_type) return;

    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }

    m_type = newType;

    glDeleteTextures(1, &m_id);

#ifndef DISABLE_MULTISAMPLE
    validateSampleCount();
#else
    // force single‐sample on WebGL
    m_samples = 1;
    // if someone passed Texture2DMultisample, silently treat as Texture2D
    if (m_type == TextureType::Texture2DMultisample)
        m_type = TextureType::Texture2D;
#endif

    glGenTextures(1, &m_id);
    allocateStorage();
    setParameters(m_minFilter, m_magFilter, m_wrapF);

    // Recreate FBO if applicable
    setupFramebuffer();
}

void RenderTexture::setSamples(uint32_t samples) {
    if (samples == m_samples) return;
    m_samples = samples;
#ifndef DISABLE_MULTISAMPLE
    validateSampleCount();
    allocateStorage();
#else
    // no-op under DISABLE_MULTISAMPLE
#endif
}

void RenderTexture::allocateStorage() {
    GLenum target = static_cast<GLenum>(m_type);
    auto [internalFmt, baseFmt, dataType] = getFormatInfo();

    glBindTexture(target, m_id);

#if !defined(DISABLE_MULTISAMPLE)
    if (m_type == TextureType::Texture2DMultisample) {
        glTexImage2DMultisample(
            GL_TEXTURE_2D_MULTISAMPLE,
            m_samples,
            internalFmt,
            m_width, m_height,
            GL_TRUE
        );
        glBindTexture(target, 0);

        int pixelSizeBytes = 4;

        if (internalFmt == GL_R8 || internalFmt == GL_RG8 || internalFmt == GL_RGB8 || internalFmt == GL_RGBA8) {
            pixelSizeBytes = 4; // Approximate for 8-bit formats
        }
        else if (internalFmt == GL_R16F || internalFmt == GL_RG16F || internalFmt == GL_RGB16F || internalFmt == GL_RGBA16F) {
            pixelSizeBytes = 8; // Approximate for 16-bit float formats
        }
        else if (internalFmt == GL_R32F || internalFmt == GL_RG32F || internalFmt == GL_RGB32F || internalFmt == GL_RGBA32F) {
            pixelSizeBytes = 16; // Approximate for 32-bit float formats
        }
        else if (internalFmt == GL_DEPTH_COMPONENT16) {
            pixelSizeBytes = 2;
        }
        else if (internalFmt == GL_DEPTH_COMPONENT24) {
            pixelSizeBytes = 3;
        }
        else if (internalFmt == GL_DEPTH_COMPONENT32F) {
            pixelSizeBytes = 4;
        }
        else if (internalFmt == GL_DEPTH24_STENCIL8) {
            pixelSizeBytes = 4;
        }
        else if (internalFmt == GL_DEPTH32F_STENCIL8) {
            pixelSizeBytes = 5;
        }

		pixelSizeBytes *= m_samples; // Multisampling increases memory usage

        ResourceStatistics::Instance().registerResource(ResourceType::RenderTexture, m_id, m_width * m_height * pixelSizeBytes); // Approximate size
        ResourceStatistics::Instance().setResourceName(ResourceType::RenderTexture, m_id, m_name);

        // Reattach to FBO if exists
        if (m_fbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, m_attachment, GL_TEXTURE_2D_MULTISAMPLE, m_id, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        return;
    }
#endif

    // single‐sample 2D or cubemap path
    if (m_type == TextureType::Texture2D) {
        glTexImage2D(GL_TEXTURE_2D,
            0,
            internalFmt,
            m_width, m_height,
            0,
            baseFmt, dataType,
            nullptr);
    }
    else if (m_type == TextureType::Cubemap) {
        for (unsigned face = 0; face < 6; ++face) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                0,
                internalFmt,
                m_width, m_height,
                0,
                baseFmt, dataType,
                nullptr);
        }
    }

    glBindTexture(target, 0);

    // Reattach to FBO if exists (for non-cubemap)
    if (m_fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, m_attachment, target, m_id, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

	int pixelSizeBytes = 4;

	if (internalFmt == GL_R8 || internalFmt == GL_RG8 || internalFmt == GL_RGB8 || internalFmt == GL_RGBA8) {
        pixelSizeBytes = 4; // Approximate for 8-bit formats
    }
    else if (internalFmt == GL_R16F || internalFmt == GL_RG16F || internalFmt == GL_RGB16F || internalFmt == GL_RGBA16F) {
        pixelSizeBytes = 8; // Approximate for 16-bit float formats
    }
    else if (internalFmt == GL_R32F || internalFmt == GL_RG32F || internalFmt == GL_RGB32F || internalFmt == GL_RGBA32F) {
        pixelSizeBytes = 16; // Approximate for 32-bit float formats
    }
    else if (internalFmt == GL_DEPTH_COMPONENT16) {
        pixelSizeBytes = 2;
    }
    else if (internalFmt == GL_DEPTH_COMPONENT24) {
        pixelSizeBytes = 3;
    }
    else if (internalFmt == GL_DEPTH_COMPONENT32F) {
        pixelSizeBytes = 4;
    }
    else if (internalFmt == GL_DEPTH24_STENCIL8) {
        pixelSizeBytes = 4;
    }
    else if (internalFmt == GL_DEPTH32F_STENCIL8) {
        pixelSizeBytes = 5;
    }

	ResourceStatistics::Instance().registerResource(ResourceType::RenderTexture, m_id, m_width * m_height * pixelSizeBytes); // Approximate size
    ResourceStatistics::Instance().setResourceName(ResourceType::RenderTexture, m_id, m_name);

}

void RenderTexture::setParameters(GLenum minFilter,
    GLenum magFilter,
    GLenum wrap)
{
    GLenum target = static_cast<GLenum>(m_type);
    glBindTexture(target, m_id);
    if (m_type != TextureType::Texture2DMultisample)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    }

    if (m_type == TextureType::Cubemap)
        glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);

    if (m_sampleDepth)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    glBindTexture(target, 0);
}

std::tuple<GLenum, GLenum, GLenum> RenderTexture::getFormatInfo() const {
    switch (m_format) {
    case TextureFormat::R8:            return { GL_R8,           GL_RED,            GL_UNSIGNED_BYTE };
    case TextureFormat::RG8:           return { GL_RG8,          GL_RG,             GL_UNSIGNED_BYTE };
    case TextureFormat::RGB8:          return { GL_RGB8,         GL_RGB,            GL_UNSIGNED_BYTE };
    case TextureFormat::RGBA8:         return { GL_RGBA8,        GL_RGBA,           GL_UNSIGNED_BYTE };
    case TextureFormat::R16F:          return { GL_R16F,         GL_RED,            GL_HALF_FLOAT };
    case TextureFormat::RG16F:         return { GL_RG16F,        GL_RG,             GL_HALF_FLOAT };
    case TextureFormat::RGB16F:        return { GL_RGB16F,       GL_RGB,            GL_HALF_FLOAT };
    case TextureFormat::RGBA16F:       return { GL_RGBA16F,      GL_RGBA,           GL_HALF_FLOAT };
    case TextureFormat::R32F:          return { GL_R32F,         GL_RED,            GL_FLOAT };
    case TextureFormat::RG32F:         return { GL_RG32F,        GL_RG,             GL_FLOAT };
    case TextureFormat::RGB32F:        return { GL_RGB32F,       GL_RGB,            GL_FLOAT };
    case TextureFormat::RGBA32F:       return { GL_RGBA32F,      GL_RGBA,           GL_FLOAT };
    case TextureFormat::Depth16:       return { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT };
    case TextureFormat::Depth24:       return { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
    case TextureFormat::Depth32F:      return { GL_DEPTH_COMPONENT32F,GL_DEPTH_COMPONENT, GL_FLOAT };
    case TextureFormat::Depth24Stencil8: return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
    case TextureFormat::Depth32FStencil8:
        return { GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV };
    }

    throw std::runtime_error("RenderTexture: unknown TextureFormat");
}

void RenderTexture::copyFrom(const RenderTexture * src)
{
    if (!src) {
        throw std::runtime_error("RenderTexture::copyFrom: source pointer is null");
    }

    if (m_width != src->width() || m_height != src->height() ||
        m_type != src->type() || m_format != src->format() ||
        m_samples != src->samples()) {
        throw std::runtime_error(
            "RenderTexture::copyFrom: source and destination must have same dimensions, type, format, and sample count");
    }

    auto [internalFmt, baseFmt, dataType] = getFormatInfo();  // Same for src and this

    GLbitfield mask;
    if (baseFmt == GL_DEPTH_STENCIL) {
        mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    }
    else if (baseFmt == GL_DEPTH_COMPONENT) {
        mask = GL_DEPTH_BUFFER_BIT;
    }
    else {
        mask = GL_COLOR_BUFFER_BIT;
    }

    GLenum filter = (mask & GL_COLOR_BUFFER_BIT) ? GL_LINEAR : GL_NEAREST;

    GLenum target = static_cast<GLenum>(m_type);

    if (m_type == TextureType::Cubemap) {
        // Use temporary FBOs for cubemaps
        GLuint fboRead = 0, fboDraw = 0;
        glGenFramebuffers(1, &fboRead);
        glGenFramebuffers(1, &fboDraw);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw);

        for (GLuint face = 0; face < 6; ++face) {
            GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, m_attachment, faceTarget, src->id(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, m_attachment, faceTarget, m_id, 0);

            if (m_attachment == GL_COLOR_ATTACHMENT0) {
                glReadBuffer(m_attachment);
                GLenum drawBuf = m_attachment;
                glDrawBuffers(1, &drawBuf);
            }

            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, mask, filter);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glDeleteFramebuffers(1, &fboRead);
        glDeleteFramebuffers(1, &fboDraw);
    }
    else {
        // Use member FBOs for non-cubemaps
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

        if (m_attachment == GL_COLOR_ATTACHMENT0) {
            glReadBuffer(m_attachment);
            GLenum drawBuf = m_attachment;
            glDrawBuffers(1, &drawBuf);
        }

        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, mask, filter);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
}

void RenderTexture::validateSampleCount() const {
#if defined(DISABLE_MULTISAMPLE)
    // no checks under WebGL
#else
    if (m_samples > 1 && m_type != TextureType::Texture2DMultisample) {
        throw std::invalid_argument(
            "Samples > 1 requires Texture2DMultisample type");
    }
#endif
}

void RenderTexture::setupFramebuffer() {
    if (m_type == TextureType::Cubemap) {
        return;  // No persistent FBO for cubemaps
    }

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    GLenum attachTarget = static_cast<GLenum>(m_type);
    glFramebufferTexture2D(GL_FRAMEBUFFER, m_attachment, attachTarget, m_id, 0);

    // Optional: Check status
    // GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // if (status != GL_FRAMEBUFFER_COMPLETE) { ... }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::SetName(std::string name)
{
    m_name = name;

	ResourceStatistics::Instance().setResourceName(ResourceType::RenderTexture, m_id, name);

}

void RenderTexture::bindFramebuffer(GLenum target) const {
    if (!m_fbo) {
        throw std::runtime_error("No FBO available for this RenderTexture (e.g., cubemap)");
    }
    glBindFramebuffer(target, m_fbo);
}