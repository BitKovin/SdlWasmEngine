#pragma once

#include <SDL2/SDL.h>
#include "gl.h"
#include <string>
#include <iostream>
#include <vector>
#include "FileSystem/FileSystem.h"

#include <includedLibraries/stb_image.h>

#include "malloc_override.h"
#include "Logger.hpp"

#include <Profiling/ResourceStatistics.hpp>

class Texture {
public:
    Texture(const std::string& filename, bool generateMipmaps = true) {
        loadFromFile(filename, generateMipmaps);
    }

    Texture() 
    {
        
    }

    // Load from compressed data (PNG/JPEG in memory)
    Texture(const unsigned char* data, size_t size, bool generateMipmaps = true) {
        loadFromMemoryCompressed(data, size, generateMipmaps);
    }

    // Load from raw pixel data (RGBA or BGRA32)
    Texture(const unsigned char* data, int width, int height, GLenum format = GL_RGBA, bool generateMipmaps = true) {
        loadFromRawData(data, width, height, format, generateMipmaps);
    }

    ~Texture() 
    {

		ResourceStatistics::Instance().unregisterResource(ResourceType::Texture, textureID);

        if (textureID != 0)
            glDeleteTextures(1, &textureID);
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    bool valid = false;

    GLuint getID() const {
        return textureID;
    }

    void setName(const std::string& name) {
        ResourceStatistics::Instance().setResourceName(ResourceType::Texture, textureID, name);
    }

private:
    GLuint textureID = 0;

    static inline bool isPowerOfTwo(int v) { return v > 0 && ((v & (v - 1)) == 0); }

    void setupTexture(int width, int height, GLenum format, const void* pixels, bool generateMipmaps) {
        if (width <= 0 || height <= 0) return;
        if (!pixels) return;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Ensure proper alignment for 3-byte RGB rows
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // On WebGL / GLES the base internalFormat must equal the format (GL_RGB/GL_RGBA).
        // Use 'format' as the internalFormat to be safe on ANGLE/WebGL and desktop.
        GLenum internalFormat = format;

        // If the uploaded pixel format is BGR/BGRA, convert to RGB/RGBA because WebGL typically doesn't accept BGR.
        const unsigned char* uploadPixels = reinterpret_cast<const unsigned char*>(pixels);
        std::vector<unsigned char> converted; // will hold converted data if needed

        // Calculate bytes per pixel based on format
        size_t bytesPerPixel = 4; // default RGBA
        switch (format) {
        case GL_RGB:
            bytesPerPixel = 3;
            break;
        case GL_RGBA:
            bytesPerPixel = 4;
            break;
        case GL_RED:
        case GL_ALPHA:
        case GL_LUMINANCE:
            bytesPerPixel = 1;
            break;
        case GL_LUMINANCE_ALPHA:
            bytesPerPixel = 2;
            break;
        case GL_RG:
            bytesPerPixel = 2;
            break;
        default:
            bytesPerPixel = 4; // fallback
            break;
        }

        // NPOT handling: WebGL1 forbids mipmaps + repeat for NPOT textures.
        bool npot = false;// !isPowerOfTwo(width) || !isPowerOfTwo(height);
        bool useMips = generateMipmaps && !npot;

        // Upload
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, uploadPixels);

        // Debug GL error right after upload (useful to catch ANGLE errors)
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "glTexImage2D failed with GL error: 0x" << std::hex << err << std::dec << std::endl;
            // still continue to set parameters (but texture may be invalid)
        }

        if (useMips) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Set sensible parameters based on NPOT/mips
        if (npot) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMips ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        if (maxAniso > 0.0f) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

        // Calculate accurate texture size
        size_t textureSize = width * height * bytesPerPixel;

        // Account for mipmaps (adds approximately 1/3 more memory)
        if (useMips) {
            textureSize += textureSize / 3;
        }

        // Create descriptive name
        std::string textureName = "Texture_" + std::to_string(width) + "x" + std::to_string(height);
        switch (format) {
        case GL_RGB:
            textureName += "_RGB";
            break;
        case GL_RGBA:
            textureName += "_RGBA";
            break;
        case GL_RED:
            textureName += "_R";
            break;
        case GL_LUMINANCE:
            textureName += "_L";
            break;
        case GL_LUMINANCE_ALPHA:
            textureName += "_LA";
            break;
        case GL_RG:
            textureName += "_RG";
            break;
        }
        if (useMips) {
            textureName += "_Mips";
        }

        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture,
            textureID,
            textureSize,
            textureName
        );

        valid = true;
    }



    void loadFromFile(const std::string& filename, bool generateMipmaps)
    {
        // 1. Read data from FileSystemEngine
        std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(filename);
        if (fileData.empty()) {
            std::cerr << "File empty or not found: " << filename << std::endl;
            return;
        }

		loadFromMemoryCompressed(fileData.data(), fileData.size(), generateMipmaps);

		ResourceStatistics::Instance().setResourceName(ResourceType::Texture, textureID, filename);
    }

    void loadFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps) {
        int width, height, channels;
        // force 4 channels (RGBA)
        unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 4);
        if (!pixels) {
            std::cerr << "Failed to load image from memory: " << stbi_failure_reason() << std::endl;
            return;
        }
        // Upload texture
        setupTexture(width, height, GL_RGBA, pixels, generateMipmaps);

        // Free the loaded image
        stbi_image_free(pixels);
    }

    void loadFromRawData(const unsigned char* data, int width, int height, GLenum format, bool generateMipmaps) {
        // quick sanity check for commonly expected RGB buffer size
        if (!data) {
            std::cerr << "loadFromRawData: null data pointer\n";
            return;
        }
        if (format == GL_RGB) {
            // If you expect RGB, verify size externally (frame vector length etc.)
            // (Cannot check here without knowing buffer size.)
        }
        setupTexture(width, height, format, data, generateMipmaps);
    }
};
