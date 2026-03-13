#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "FileSystem/FileSystem.h"

#include <includedLibraries/stb_image.h>
#include <includedLibraries/stb_image_resize.h>   // add this include
#include <bgfx/bgfx.h>

#include "malloc_override.h"
#include "Logger.hpp"
#include <Profiling/ResourceStatistics.hpp>

class Texture {
public:
    Texture(const std::string& filename, bool generateMipmaps = true) {
        loadFromFile(filename, generateMipmaps);
    }

    Texture() {}

    // Load from compressed data (PNG/JPEG in memory)
    Texture(const unsigned char* data, size_t size, bool generateMipmaps = true) {
        loadFromMemoryCompressed(data, size, generateMipmaps);
    }

    // Load from raw pixel data (RGBA or BGRA)
    Texture(const unsigned char* data, int width, int height, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8, bool generateMipmaps = true) {
        loadFromRawData(data, width, height, format, generateMipmaps);
    }

    ~Texture() {
        ResourceStatistics::Instance().unregisterResource(ResourceType::Texture, m_handle.idx);

        if (bgfx::isValid(m_handle))
            bgfx::destroy(m_handle);
    }

    void bind(uint8_t stage, bgfx::UniformHandle sampler) const {
        bgfx::setTexture(stage, sampler, m_handle);
    }

    bool valid = false;
    int  width = 0;
    int  height = 0;

    bgfx::TextureHandle getHandle() const {
        return m_handle;
    }

    uint16_t getID() const
    {
        if (bgfx::isValid(m_handle))
            return m_handle.idx;
        else
            return 0;
    }

    bgfx::TextureHandle getTextureHandle() const {
        return m_handle;
    }

    void setName(const std::string& name) {
        ResourceStatistics::Instance().setResourceName(ResourceType::Texture, m_handle.idx, name);
        if (bgfx::isValid(m_handle))
            bgfx::setName(m_handle, name.c_str(), (int32_t)name.size());
    }

private:
    bgfx::TextureHandle m_handle = BGFX_INVALID_HANDLE;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    static uint64_t buildFlags() {
        return BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
    }

    static size_t bytesPerPixel(bgfx::TextureFormat::Enum fmt) {
        switch (fmt) {
        case bgfx::TextureFormat::R8:    return 1;
        case bgfx::TextureFormat::RG8:   return 2;
        case bgfx::TextureFormat::RGB8:  return 3;
        case bgfx::TextureFormat::RGBA8: return 4;
        case bgfx::TextureFormat::BGRA8: return 4;
        default:                         return 4;
        }
    }

    static std::string formatSuffix(bgfx::TextureFormat::Enum fmt) {
        switch (fmt) {
        case bgfx::TextureFormat::R8:    return "_R";
        case bgfx::TextureFormat::RG8:   return "_RG";
        case bgfx::TextureFormat::RGB8:  return "_RGB";
        case bgfx::TextureFormat::RGBA8: return "_RGBA";
        case bgfx::TextureFormat::BGRA8: return "_BGRA";
        default:                         return "_Unknown";
        }
    }

    // -----------------------------------------------------------------------
    // Core upload
    // -----------------------------------------------------------------------
    void setupTexture(int w, int h,
        bgfx::TextureFormat::Enum format,
        const void* pixels,
        bool generateMipmaps)
    {
        if (w <= 0 || h <= 0 || !pixels)
        {
            std::cerr << "setupTexture: invalid arguments ("
                << w << "x" << h << ", pixels="
                << (pixels ? "ok" : "null") << ")\n";
            return;
        }

        // Calculate how many mip levels fit (down to 1x1).
        const bool hasMips = generateMipmaps && (w > 1 || h > 1);
        const uint8_t numMips = hasMips
            ? (uint8_t)(1 + (int)std::floor(std::log2((double)std::max(w, h))))
            : 1;

        // Allocate the texture with mip storage but no initial data —
        // we upload each level individually with updateTexture2D below.
        m_handle = bgfx::createTexture2D(
            (uint16_t)w,
            (uint16_t)h,
            hasMips,
            1,         // numLayers
            format,
            buildFlags()
        );

        if (!bgfx::isValid(m_handle)) {
            std::cerr << "bgfx::createTexture2D failed (" << w << "x" << h << ")\n";
            return;
        }

        // Upload mip 0 — the original pixels.
        {
            const uint32_t sz = (uint32_t)(w * h * bytesPerPixel(format));
            bgfx::updateTexture2D(m_handle, 0, 0,
                0, 0, (uint16_t)w, (uint16_t)h,
                bgfx::copy(pixels, sz));
        }

        // Generate and upload remaining mip levels via stbir_resize_uint8.
        // Each level is downsampled from the previous one so quality degrades
        // gracefully (progressive box filter) rather than resampling from
        // the full-res image every time.
        if (hasMips) {
            const int bpp = (int)bytesPerPixel(format);

            std::vector<uint8_t> prevPixels(
                (const uint8_t*)pixels,
                (const uint8_t*)pixels + w * h * bpp);

            int mipW = w, mipH = h;

            for (uint8_t mip = 1; mip < numMips; ++mip) {
                const int dstW = std::max(1, mipW / 2);
                const int dstH = std::max(1, mipH / 2);

                std::vector<uint8_t> mipPixels((size_t)dstW * dstH * bpp);

                stbir_resize_uint8(
                    prevPixels.data(), mipW, mipH, 0,
                    mipPixels.data(), dstW, dstH, 0,
                    bpp);

                bgfx::updateTexture2D(m_handle, 0, mip,
                    0, 0, (uint16_t)dstW, (uint16_t)dstH,
                    bgfx::copy(mipPixels.data(), (uint32_t)mipPixels.size()));

                prevPixels = std::move(mipPixels);
                mipW = dstW;
                mipH = dstH;
            }
        }

        width = w;
        height = h;

        const size_t textureSize = (size_t)w * h * bytesPerPixel(format);
        std::string textureName = "Texture_"
            + std::to_string(w) + "x" + std::to_string(h)
            + formatSuffix(format);

        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture, m_handle.idx, textureSize, textureName);

        valid = true;
    }

    // -----------------------------------------------------------------------
    // Load paths
    // -----------------------------------------------------------------------
    void loadFromFile(const std::string& filename, bool generateMipmaps) {
        std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(filename);
        if (fileData.empty()) {
            std::cerr << "Texture: file empty or not found: " << filename << "\n";
            return;
        }

        loadFromMemoryCompressed(fileData.data(), fileData.size(), generateMipmaps);

        if (bgfx::isValid(m_handle)) {
            bgfx::setName(m_handle, filename.c_str(), (int32_t)filename.size());
            ResourceStatistics::Instance().setResourceName(ResourceType::Texture, m_handle.idx, filename);
        }
    }

    void loadFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps) {
        if (!data || size == 0) {
            std::cerr << "Texture: null or empty compressed data\n";
            return;
        }

        int w, h, channels;
        unsigned char* pixels = stbi_load_from_memory(
            data, (int)size, &w, &h, &channels, 4);

        if (!pixels) {
            std::cerr << "Texture: stbi_load_from_memory failed: " << stbi_failure_reason() << "\n";
            return;
        }

        setupTexture(w, h, bgfx::TextureFormat::RGBA8, pixels, generateMipmaps);
        stbi_image_free(pixels);
    }

    void loadFromRawData(const unsigned char* data, int w, int h,
        bgfx::TextureFormat::Enum format, bool generateMipmaps)
    {
        if (!data) {
            std::cerr << "Texture: loadFromRawData called with null data pointer\n";
            return;
        }
        setupTexture(w, h, format, data, generateMipmaps);
    }
};