#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <iostream>
#include <vector>
#include "FileSystem/FileSystem.h"

#include "stb_lib/stb_image.h"

#include "malloc_override.h"
#include "Logger.hpp"

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

    ~Texture() {
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
        return;

        // 2. Allocate a buffer that SDL owns
        SDL_RWops* rw = SDL_RWFromConstMem(fileData.data(), static_cast<int>(fileData.size()));
        if (!rw) {
            Logger::Log(filename);
            SDL_Log("Failed to create RWops: %s", SDL_GetError());
            return;
        }

        // 3. Load the surface from RWops
        SDL_Surface* surface = IMG_Load_RW(rw, 1); // SDL frees RWops automatically
        if (!surface) {
            Logger::Log(filename);
            SDL_Log("IMG_Load_RW failed: %s", IMG_GetError());
            return;
        }

        // 4. Convert to RGBA32
        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        if (!converted) {
            std::cerr << "Failed to convert surface: " << SDL_GetError() << std::endl;
            return;
        }

        // 5. Upload to GPU
        setupTexture(converted->w, converted->h, GL_RGBA, converted->pixels, generateMipmaps);
        SDL_FreeSurface(converted);
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
