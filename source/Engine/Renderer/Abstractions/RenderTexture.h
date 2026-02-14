#pragma once
#include "../../gl.h"
#include <stdexcept>
#include <unordered_map>

enum class TextureFormat {
    // Color formats
    R8 = GL_R8, RG8 = GL_RG8, RGB8 = GL_RGB8, RGBA8 = GL_RGBA8,
    R16F = GL_R16F, RG16F = GL_RG16F, RGB16F = GL_RGB16F, RGBA16F = GL_RGBA16F,
    R32F = GL_R32F, RG32F = GL_RG32F, RGB32F = GL_RGB32F, RGBA32F = GL_RGBA32F,

    // Depth/stencil
    Depth16 = GL_DEPTH_COMPONENT16,
    Depth24 = GL_DEPTH_COMPONENT24,
    Depth32F = GL_DEPTH_COMPONENT32F,
    Depth24Stencil8 = GL_DEPTH24_STENCIL8,
    Depth32FStencil8 = GL_DEPTH32F_STENCIL8
};

enum class TextureType {
    Texture2D = GL_TEXTURE_2D,
    Cubemap = GL_TEXTURE_CUBE_MAP,
    Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE
};

class RenderTexture {
public:
    // Add samples parameter to constructor
    RenderTexture(uint32_t width, uint32_t height, TextureFormat format,
        TextureType type = TextureType::Texture2D, bool sampleDepth = false,
        GLenum minFilter = GL_LINEAR,
        GLenum magFilter = GL_LINEAR,
        GLenum wrap = GL_CLAMP_TO_EDGE,
        uint32_t samples = 1);  // New parameter

    ~RenderTexture();

    void bind(uint32_t unit = 0) const;
    void unbind() const;
    bool resize(uint32_t width, uint32_t height);

    void setTextureType(TextureType newType);

    // Add sample management
    void setSamples(uint32_t samples);
    uint32_t samples() const { return m_samples; }

    GLuint id() const { return m_id; }
    TextureFormat format() const { return m_format; }
    TextureType type() const { return m_type; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    void copyFrom(const RenderTexture* src);

    // New methods for FBO management
    GLuint framebuffer() const { return m_fbo; }
    void bindFramebuffer(GLenum target = GL_FRAMEBUFFER) const;

    void SetName(std::string name);

private:
    GLuint m_id;
    GLuint m_fbo = 0;
    uint32_t m_width, m_height;
    TextureFormat m_format;
    TextureType m_type;
    uint32_t m_samples;  // Add sample count

    GLenum m_minFilter;
    GLenum m_magFilter;
    GLenum m_wrapF;

    std::string m_name = "";

    bool m_sampleDepth = false;

    GLenum m_attachment;

    void allocateStorage();
    void setParameters(GLenum minFilter, GLenum magFilter, GLenum wrap);
    std::tuple<GLenum, GLenum, GLenum> getFormatInfo() const;
    void validateSampleCount() const;
    void setupFramebuffer();



};