#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <vector>
#include <iostream>

class CubemapTexture {
public:
    // faces should be provided in this order:
    // right, left, top, bottom, front, back
    CubemapTexture(const std::vector<std::string>& faces, bool generateMipmaps = false) {
        if (faces.size() != 6) {
            std::cerr << "Cubemap texture requires exactly 6 faces." << std::endl;
            return;
        }
        loadFromFiles(faces, generateMipmaps);
    }

    ~CubemapTexture() {
        glDeleteTextures(1, &textureID);
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    }

    GLuint getID() const {
        return textureID;
    }

private:
    GLuint textureID = 0;

    void loadFromFiles(const std::vector<std::string>& faces, bool generateMipmaps) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        for (unsigned int i = 0; i < faces.size(); i++) {
            SDL_Surface* surface = IMG_Load(faces[i].c_str());
            if (!surface) {
                std::cerr << "Error loading cubemap face (" << faces[i] << "): "
                    << IMG_GetError() << std::endl;
                continue;
            }

            // Convert surface to a consistent pixel format (RGBA)
            SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(surface);
            if (!converted_surface) {
                std::cerr << "Error converting surface (" << faces[i] << "): "
                    << SDL_GetError() << std::endl;
                continue;
            }

            // Load the texture data into the cubemap face.
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                converted_surface->w, converted_surface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, converted_surface->pixels);

            SDL_FreeSurface(converted_surface);
        }

        // Set texture parameters for the cubemap.
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
            generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
    }
};