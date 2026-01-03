#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "gl.h"
#include <string>
#include <vector>
#include <iostream>

#include "malloc_override.h"

#include "Helpers/StringHelper.h"

class CubemapTexture 
{

private:

    // Helper: rotate an SDL_Surface 90° CW or CCW
    static SDL_Surface* rotate90(SDL_Surface* src, bool clockwise) {
        // Create a destination surface of swapped dimensions
        SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(
            0, src->h, src->w, src->format->BitsPerPixel, src->format->format);
        if (!dst) return nullptr;

        SDL_LockSurface(src);
        SDL_LockSurface(dst);

        int sw = src->w, sh = src->h;
        int dw = dst->w, dh = dst->h;
        int bpp = src->format->BytesPerPixel;

        Uint8* sp = (Uint8*)src->pixels;
        Uint8* dp = (Uint8*)dst->pixels;
        int spitch = src->pitch;
        int dpitch = dst->pitch;

        for (int y = 0; y < sh; ++y) {
            for (int x = 0; x < sw; ++x) {
                // read pixel
                Uint8* srcp = sp + y * spitch + x * bpp;

                // compute dest coords
                int dx = clockwise ? y : (sh - 1 - y);
                int dy = clockwise ? (sw - 1 - x) : x;

                Uint8* dstp = dp + dy * dpitch + dx * bpp;
                memcpy(dstp, srcp, bpp);
            }
        }

        SDL_UnlockSurface(src);
        SDL_UnlockSurface(dst);
        return dst;
    }

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

    CubemapTexture(const std::string& base, bool generateMipmaps = false) {
        

        std::vector<std::string> faces;

		faces.push_back(StringHelper::Replace(base, ".", "_lf."));
		faces.push_back(StringHelper::Replace(base, ".", "_rt."));
        faces.push_back(StringHelper::Replace(base, ".", "_up."));
        faces.push_back(StringHelper::Replace(base, ".", "_dn."));
        faces.push_back(StringHelper::Replace(base, ".", "_ft."));
        faces.push_back(StringHelper::Replace(base, ".", "_bk."));

        loadFromFiles(faces, false);
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

    void loadFromFiles(const std::vector<std::string>& faces, bool generateMipmaps) 
    {
        if (faces.size() != 6) {
            std::cerr << "[Cubemap] Need 6 faces, got " << faces.size() << std::endl;
            return;
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned i = 0; i < 6; ++i) 
        {

            auto fileData = FileSystemEngine::ReadFileBinary(faces[i].c_str());

            // Create an SDL_RWops from memory
            SDL_RWops* rw = SDL_RWFromConstMem(fileData.data(), static_cast<int>(fileData.size()));
            if (!rw) {
                SDL_Log("Failed to create RWops: %s", SDL_GetError());
                return;
            }

            // Load the image from RWops
            SDL_Surface* surf = IMG_Load_RW(rw, 1); // 1 = automatically free the RWops after loading
            if (!surf) {
                SDL_Log("IMG_Load_RW failed: %s", IMG_GetError());
            }

            SDL_Surface* conv = SDL_ConvertSurfaceFormat(
                surf, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(surf);
            if (!conv) {
                std::cerr << "[Cubemap] Convert failed " << faces[i]
                    << ": " << SDL_GetError() << std::endl;
                    continue;
            }

            // rotate +Y (index 2) 90° CW, –Y (index 3) 90° CCW
            if (i == 2 || i == 3) {
                bool cw = (i == 2);
                SDL_Surface* rot = rotate90(conv, !cw);
                SDL_FreeSurface(conv);
                if (!rot) {
                    std::cerr << "[Cubemap] Rotation failed for "
                        << faces[i] << std::endl;
                    continue;
                }
                conv = rot;
            }

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGBA,
                conv->w, conv->h,
                0, GL_RGBA, GL_UNSIGNED_BYTE,
                conv->pixels
            );
            SDL_FreeSurface(conv);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
            generateMipmaps
            ? GL_LINEAR_MIPMAP_LINEAR
            : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
    }


};