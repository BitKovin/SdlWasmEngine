#pragma once

#include <SDL2/SDL.h>
#include "gl.h"
#include <string>
#include <vector>
#include <iostream>

#include "malloc_override.h"

#include "Helpers/StringHelper.h"

#include <Profiling/ResourceStatistics.hpp>

class CubemapTexture 
{

private:

    // Helper: rotate an SDL_Surface 90° CW or CCW
// Allocates and returns a new RGBA buffer rotated 90° CW (cw=true) or CCW.
// Caller must free with stbi_image_free().
    static stbi_uc* rotate90_rgba(const stbi_uc* src, int w, int h, bool cw)
    {
        stbi_uc* dst = static_cast<stbi_uc*>(malloc(w * h * 4));
        if (!dst) return nullptr;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const stbi_uc* s = src + (y * w + x) * 4;
                stbi_uc* d;
                if (cw)
                    d = dst + (x * h + (h - 1 - y)) * 4; // CW:  dst(x, h-1-y)
                else
                    d = dst + ((w - 1 - x) * h + y) * 4; // CCW: dst(w-1-x, y)
                d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
            }
        }
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
		ResourceStatistics::Instance().unregisterResource(ResourceType::TextureCube, textureID);
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

        size_t totalSize = 0;
        int faceWidth = 0;
        int faceHeight = 0;

        for (unsigned i = 0; i < 6; ++i)
        {
            auto fileData = FileSystemEngine::ReadFileBinary(faces[i].c_str());

            int width = 0, height = 0, channels = 0;
            stbi_uc* pixels = stbi_load_from_memory(
                reinterpret_cast<const stbi_uc*>(fileData.data()),
                static_cast<int>(fileData.size()),
                &width, &height, &channels,
                STBI_rgb_alpha
            );

            if (!pixels) {
                std::cerr << "[Cubemap] stbi_load_from_memory failed for "
                    << faces[i] << ": " << stbi_failure_reason() << std::endl;
                continue;
            }

            // rotate +Y (index 2) 90° CW, –Y (index 3) 90° CCW
            if (i == 2 || i == 3) {
                bool cw = (i == 2);
                stbi_uc* rotated = rotate90_rgba(pixels, width, height, cw);
                stbi_image_free(pixels);
                if (!rotated) {
                    std::cerr << "[Cubemap] Rotation failed for "
                        << faces[i] << std::endl;
                    continue;
                }
                pixels = rotated;
                // width and height swap after 90° rotation
                std::swap(width, height);
            }

            // Store dimensions from first face
            if (i == 0) {
                faceWidth = width;
                faceHeight = height;
            }

            const size_t faceSize = static_cast<size_t>(width) * height * 4;
            totalSize += faceSize;

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGBA,
                width, height,
                0, GL_RGBA, GL_UNSIGNED_BYTE,
                pixels
            );

            stbi_image_free(pixels);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
            generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            totalSize += totalSize / 3;
        }

        // Find common prefix of all face paths
        std::string resourceName = "Cubemap";
        if (!faces.empty()) {
            resourceName = faces[0];
            for (size_t i = 1; i < faces.size(); ++i) {
                size_t minLen = std::min(resourceName.length(), faces[i].length());
                size_t j = 0;
                while (j < minLen && resourceName[j] == faces[i][j])
                    ++j;
                resourceName = resourceName.substr(0, j);
            }
            if (resourceName.empty() || resourceName.length() < 3)
                resourceName = "Cubemap";
        }

        ResourceStatistics::Instance().registerResource(
            ResourceType::TextureCube,
            textureID,
            totalSize,
            resourceName
        );
    }


};