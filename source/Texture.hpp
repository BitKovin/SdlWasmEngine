#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <iostream>

class Texture {
public:
    Texture(const std::string& filename, bool generateMipmaps = false) {
        loadFromFile(filename, generateMipmaps);
    }

    ~Texture() {
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

    void loadFromFile(const std::string& filename, bool generateMipmaps) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Error loading image: " << IMG_GetError() << std::endl;
            return;
        }

        SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        if (!converted_surface) {
            std::cerr << "Error converting surface: " << SDL_GetError() << std::endl;
            return;
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted_surface->w, converted_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted_surface->pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        SDL_FreeSurface(converted_surface);

        valid = true;

    }
};
