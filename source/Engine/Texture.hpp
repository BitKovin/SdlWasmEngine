#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "FileSystem/FileSystem.h"
#include <includedLibraries/stb_image.h>
#include <includedLibraries/stb_image_resize.h> // add this include
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
    Texture(const unsigned char* data, size_t size, bool generateMipmaps = true) {
        loadFromMemoryCompressed(data, size, generateMipmaps);
    }
    Texture(const unsigned char* data, int width, int height,
        bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8,
        bool generateMipmaps = true) {
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
    int width = 0;
    int height = 0;
    bool transparent = false;
    bgfx::TextureHandle getHandle() const { return m_handle; }
    bgfx::TextureHandle getTextureHandle() const { return m_handle; }
    uint16_t getID() const {
        return bgfx::isValid(m_handle) ? m_handle.idx : 0;
    }
    void setName(const std::string& name) {
        ResourceStatistics::Instance().setResourceName(ResourceType::Texture, m_handle.idx, name);
        if (bgfx::isValid(m_handle))
            bgfx::setName(m_handle, name.c_str(), (int32_t)name.size());
    }

    glm::vec3 SampleRGB(float u, float v) const
    {
        if (!valid || m_pixels.empty() || width <= 0 || height <= 0)
            return glm::vec3(0.0f);

        // Wrap UVs (repeat)
        u = u - std::floor(u);
        v = v - std::floor(v);

        // Convert to pixel space
        float x = u * (width - 1);
        float y = v * (height - 1);

        int x0 = (int)x;
        int y0 = (int)y;
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        float tx = x - x0;
        float ty = y - y0;

        auto sample = [&](int px, int py) -> glm::vec3
            {
                const uint8_t* p = &m_pixels[(py * width + px) * m_bpp];
                return glm::vec3(
                    p[0] / 255.0f,
                    p[1] / 255.0f,
                    p[2] / 255.0f
                );
            };

        glm::vec3 c00 = sample(x0, y0);
        glm::vec3 c10 = sample(x1, y0);
        glm::vec3 c01 = sample(x0, y1);
        glm::vec3 c11 = sample(x1, y1);

        // Bilinear interpolation
        glm::vec3 cx0 = glm::mix(c00, c10, tx);
        glm::vec3 cx1 = glm::mix(c01, c11, tx);

        return glm::mix(cx0, cx1, ty);
    }

private:
    bgfx::TextureHandle m_handle = BGFX_INVALID_HANDLE;

    std::vector<uint8_t> m_pixels; // CPU copy (RGBA8 assumed)
    int m_bpp = 4; // bytes per pixel

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    static uint64_t buildFlags() {
        return BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
    }
    static size_t bytesPerPixel(bgfx::TextureFormat::Enum fmt) {
        switch (fmt) {
        case bgfx::TextureFormat::R8: return 1;
        case bgfx::TextureFormat::RG8: return 2;
        case bgfx::TextureFormat::RGB8: return 3;
        case bgfx::TextureFormat::RGBA8: return 4;
        case bgfx::TextureFormat::BGRA8: return 4;
        default: return 4;
        }
    }
    static std::string formatSuffix(bgfx::TextureFormat::Enum fmt) {
        switch (fmt) {
        case bgfx::TextureFormat::R8: return "_R";
        case bgfx::TextureFormat::RG8: return "_RG";
        case bgfx::TextureFormat::RGB8: return "_RGB";
        case bgfx::TextureFormat::RGBA8: return "_RGBA";
        case bgfx::TextureFormat::BGRA8: return "*BGRA";
        default: return "*Unknown";
        }
    }
    static int nextPow2(int x) { int p = 1; while (p < x) p <<= 1; return p; }
    static bool isPow2(int x) { return x > 0 && (x & (x - 1)) == 0; }
    // Downsample src → dst using Catmull-Rom.
    // Always samples from the full POT base (src), never from a previous mip,
    // so filter error does not compound across levels.
    static void downsample(
        const uint8_t* src, int srcW, int srcH,
        uint8_t* dst, int dstW, int dstH,
        int bpp)
    {
        stbir_resize_uint8_generic(
            src, srcW, srcH, 0,
            dst, dstW, dstH, 0,
            bpp,
            bpp == 4 ? 3 : STBIR_ALPHA_CHANNEL_NONE, // alpha channel index
            bpp == 4 ? STBIR_FLAG_ALPHA_PREMULTIPLIED : 0,
            STBIR_EDGE_CLAMP,
            STBIR_FILTER_BOX, // sharp, no box-blur compounding
            STBIR_COLORSPACE_LINEAR,
            nullptr);
    }
    static void downsample2x2Box(
        const uint8_t* src, int srcW, int srcH,
        uint8_t* dst, int dstW, int dstH,
        int bpp)
    {
        for (int y = 0; y < dstH; ++y) {
            for (int x = 0; x < dstW; ++x) {
                for (int c = 0; c < bpp; ++c) {
                    int sx = x * 2;
                    int sy = y * 2;
                    // Average 2x2 block
                    int p00 = src[(sy * srcW + sx) * bpp + c];
                    int p10 = (sx + 1 < srcW) ? src[(sy * srcW + sx + 1) * bpp + c] : p00;
                    int p01 = (sy + 1 < srcH) ? src[((sy + 1) * srcW + sx) * bpp + c] : p00;
                    int p11 = (sx + 1 < srcW && sy + 1 < srcH) ?
                        src[((sy + 1) * srcW + sx + 1) * bpp + c] : p00;
                    dst[(y * dstW + x) * bpp + c] = (uint8_t)((p00 + p10 + p01 + p11) / 4);
                }
            }
        }
    }
    void setupTexture_VeryFastMips(int w, int h,
        bgfx::TextureFormat::Enum format,
        const void* pixels,
        bool generateMipmaps)
    {
        if (w <= 0 || h <= 0 || !pixels) {
            std::cerr << "setupTexture: invalid arguments\n";
            return;
        }
        const int bpp = (int)bytesPerPixel(format);
        const bool hasMips = generateMipmaps && (w > 1 || h > 1);
        const uint8_t numMips = hasMips
            ? (uint8_t)(1 + (int)std::floor(std::log2((double)std::max(w, h))))
            : 1;
        m_handle = bgfx::createTexture2D(
            (uint16_t)w, (uint16_t)h,
            hasMips, 1, format, buildFlags());
        if (!bgfx::isValid(m_handle)) {
            std::cerr << "bgfx::createTexture2D failed\n";
            return;
        }
        // Upload mip 0
        bgfx::updateTexture2D(m_handle, 0, 0,
            0, 0, (uint16_t)w, (uint16_t)h,
            bgfx::copy(pixels, (uint32_t)(w * h * bpp)));

        // Generate mipmaps with simple 2x2 box filter.
        // After the loop, srcMip holds the last (smallest) mip — we use it
        // for a cheap transparency scan instead of walking the full mip 0.
        if (hasMips) {
            std::vector<uint8_t> srcMip((const uint8_t*)pixels,
                (const uint8_t*)pixels + w * h * bpp);
            int currentW = w;
            int currentH = h;
            for (uint8_t mip = 1; mip < numMips; ++mip) {
                const int dstW = std::max(1, currentW / 2);
                const int dstH = std::max(1, currentH / 2);
                std::vector<uint8_t> dstMip((size_t)dstW * dstH * bpp);
                // Simple 2x2 box filter
                downsample2x2Box(srcMip.data(), currentW, currentH,
                    dstMip.data(), dstW, dstH, bpp);
                bgfx::updateTexture2D(m_handle, 0, mip,
                    0, 0, (uint16_t)dstW, (uint16_t)dstH,
                    bgfx::copy(dstMip.data(), (uint32_t)dstMip.size()));
                srcMip = std::move(dstMip);
                currentW = dstW;
                currentH = dstH;
            }

            // Detect transparency from the last (smallest) mip.
            // It is only a few pixels, so the scan is essentially free.
            // Averaged alpha < 255 means at least one source region contained
            // a non-opaque pixel, which is the right threshold for enabling
            // alpha blending on the draw call side.
            if (bpp == 4) { // only RGBA8 / BGRA8 carry an alpha channel
                const int alphaIdx = 3; // byte 3 is A in both RGBA8 and BGRA8
                const size_t pixelCount = srcMip.size() / bpp;
                for (size_t i = 0; i < pixelCount; ++i) {
                    if (srcMip[i * bpp + alphaIdx] < 255) {
                        transparent = true;
                        break;
                    }
                }
            }
        }
        else if (bpp == 4) {
            // No mips generated (1×1 texture or mipmaps disabled).
            // Scan mip 0 directly — still negligibly small in the common case.
            const uint8_t* p = (const uint8_t*)pixels;
            const size_t pixelCount = (size_t)w * h;
            for (size_t i = 0; i < pixelCount; ++i) {
                if (p[i * bpp + 3] < 255) {
                    transparent = true;
                    break;
                }
            }
        }

        m_pixels.assign((const uint8_t*)pixels,
            (const uint8_t*)pixels + (size_t)w * h * bpp);
        m_bpp = bpp;

        width = w;
        height = h;
        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture, m_handle.idx,
            (size_t)w * h * bpp,
            "Texture*" + std::to_string(w) + "x" + std::to_string(h) + formatSuffix(format));
        valid = true;
    }


    // -----------------------------------------------------------------------
    // Core upload
    // -----------------------------------------------------------------------
    void setupTexture(int w, int h,
        bgfx::TextureFormat::Enum format,
        const void* pixels,
        bool generateMipmaps)
    {
        if (w <= 0 || h <= 0 || !pixels) {
            std::cerr << "setupTexture: invalid arguments ("
                << w << "x" << h << ", pixels="
                << (pixels ? "ok" : "null") << ")\n";
            return;
        }
        setupTexture_VeryFastMips(w, h, format, pixels, generateMipmaps);
        return;
        const int bpp = (int)bytesPerPixel(format);
        // -----------------------------------------------------------------------
        // Step 1 — upscale to POT if needed.
        // Mip chains require POT dimensions so each half-step lands on an integer.
        // -----------------------------------------------------------------------
        const int pot_w = isPow2(w) ? w : nextPow2(w);
        const int pot_h = isPow2(h) ? h : nextPow2(h);
        const bool needsUpscale = generateMipmaps && (pot_w != w || pot_h != h) && false;
        std::vector<uint8_t> pot_pixels;
        const uint8_t* basePixels = (const uint8_t*)pixels;
        int base_w = w, base_h = h;
        if (needsUpscale) {
            pot_pixels.resize((size_t)pot_w * pot_h * bpp);
            // Upscale with Catmull-Rom to preserve sharpness at the base level.
            stbir_resize_uint8_generic(
                (const uint8_t*)pixels, w, h, 0,
                pot_pixels.data(), pot_w, pot_h, 0,
                bpp,
                bpp == 4 ? 3 : STBIR_ALPHA_CHANNEL_NONE,
                bpp == 4 ? STBIR_FLAG_ALPHA_PREMULTIPLIED : 0,
                STBIR_EDGE_CLAMP,
                STBIR_FILTER_DEFAULT,
                STBIR_COLORSPACE_LINEAR,
                nullptr);
            basePixels = pot_pixels.data();
            base_w = pot_w;
            base_h = pot_h;
        }
        // -----------------------------------------------------------------------
        // Step 2 — allocate texture with full mip chain.
        // -----------------------------------------------------------------------
        const bool hasMips = generateMipmaps && (base_w > 1 || base_h > 1);
        const uint8_t numMips = hasMips
            ? (uint8_t)(1 + (int)std::floor(std::log2((double)std::max(base_w, base_h))))
            : 1;
        m_handle = bgfx::createTexture2D(
            (uint16_t)base_w, (uint16_t)base_h,
            hasMips, 1, format, buildFlags());
        if (!bgfx::isValid(m_handle)) {
            std::cerr << "bgfx::createTexture2D failed (" << base_w << "x" << base_h << ")\n";
            return;
        }
        // -----------------------------------------------------------------------
        // Step 3 — upload mip 0.
        // -----------------------------------------------------------------------
        bgfx::updateTexture2D(m_handle, 0, 0,
            0, 0, (uint16_t)base_w, (uint16_t)base_h,
            bgfx::copy(basePixels, (uint32_t)(base_w * base_h * bpp)));
        // -----------------------------------------------------------------------
        // Step 4 — generate every mip by downsampling from the POT base.
        //
        // Sampling each level directly from base (not from the previous mip)
        // means the Catmull-Rom filter runs once per level against clean data.
        // Progressive sampling (prev→next) would apply the filter log2(N) times
        // to the smallest levels, which is what caused the early blur.
        // -----------------------------------------------------------------------
        if (hasMips) {
            // Start with a working copy of the base level (mip 0)
            // One-time memcpy is negligible compared to the previous O(N·log N) cost
            std::vector<uint8_t> workingMip(
                basePixels,
                basePixels + (size_t)base_w * base_h * bpp);
            int currentW = base_w;
            int currentH = base_h;
            for (uint8_t mip = 1; mip < numMips; ++mip) {
                const int dstW = std::max(1, currentW / 2);
                const int dstH = std::max(1, currentH / 2);
                std::vector<uint8_t> mipPixels((size_t)dstW * dstH * bpp);
                downsample(workingMip.data(), currentW, currentH,
                    mipPixels.data(), dstW, dstH, bpp);
                bgfx::updateTexture2D(m_handle, 0, mip,
                    0, 0, (uint16_t)dstW, (uint16_t)dstH,
                    bgfx::copy(mipPixels.data(), (uint32_t)mipPixels.size()));
                // Next iteration works from this level
                workingMip = std::move(mipPixels);
                currentW = dstW;
                currentH = dstH;
            }
        }
        // Report original dimensions to callers.
        width = w;
        height = h;
        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture, m_handle.idx,
            (size_t)w * h * bpp,
            "Texture*" + std::to_string(w) + "x" + std::to_string(h) + formatSuffix(format));
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
            ResourceStatistics::Instance().setResourceName(
                ResourceType::Texture, m_handle.idx, filename);
        }
    }
    void loadFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps) {
        if (!data || size == 0) {
            std::cerr << "Texture: null or empty compressed data\n";
            return;
        }
        int w, h, channels;
        unsigned char* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
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