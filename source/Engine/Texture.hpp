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
#include "AssetLoadState.h"
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

    // Readiness for AssetRegistry's async loading - see AssetLoadState.h.
    // A Texture only ever moves None <-> Visual, there's no CPU-only "Logic"
    // stage the way there is for a skinned model's bones.
    AssetLoadState loadState;

    // Plain decoded pixels, produced off the main thread (Loader thread, or
    // wherever else needs one - e.g. an embedded GLB texture found while
    // parsing a model). Holds no bgfx handle, so it's safe to build anywhere;
    // only UploadDecoded() below is main-thread-only.
    struct Decoded
    {
        std::vector<uint8_t> pixels; // tightly packed RGBA8, mip level 0

        // Every mip below level 0, already downsampled - box-filtered on
        // whichever thread produced this Decoded (the Loader thread, for
        // the normal async path - see AssetRegistry::QueueTextureUploadJob).
        // UploadDecoded()/UploadDecodedMips() below just hand these straight
        // to bgfx, so no per-pixel filtering work is left for the main
        // thread to do. Empty when generateMipmaps was false or the image
        // is 1x1. Filled in by BuildMipsAndTransparency().
        std::vector<std::vector<uint8_t>> mipLevels;

        int  width           = 0;
        int  height          = 0;
        bool generateMipmaps = true;

        // Also computed off-thread by BuildMipsAndTransparency(), so
        // UploadDecodedMips() can just copy it onto the live Texture instead
        // of re-scanning pixels on the main thread.
        bool transparent     = false;
        bool valid           = false;
    };

    static Decoded DecodeFromFile(const std::string& filename, bool generateMipmaps)
    {
        std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(filename);
        if (fileData.empty())
        {
            Logger::Error("Texture: file empty or not found: %s", filename.c_str());
            return {};
        }
        return DecodeFromMemoryCompressed(fileData.data(), fileData.size(), generateMipmaps);
    }

    static Decoded DecodeFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps)
    {
        Decoded out;
        if (!data || size == 0)
        {
            Logger::Error("Texture: null or empty compressed data");
            return out;
        }
        int w, h, channels;
        unsigned char* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
        if (!pixels)
        {
            Logger::Error("Texture: stbi_load_from_memory failed: %s", stbi_failure_reason());
            return out;
        }
        out.pixels.assign(pixels, pixels + (size_t)w * h * 4);
        out.width = w;
        out.height = h;
        out.generateMipmaps = generateMipmaps;
        out.valid = true;
        stbi_image_free(pixels);

        // All CPU work for this image (box-filtering every mip level, the
        // transparency scan) happens right here, before this Decoded ever
        // crosses back to the main thread - see Decoded::mipLevels.
        BuildMipsAndTransparency(out);
        return out;
    }

    // Same data an embedded (uncompressed) GLB texture already carries -
    // just wrapped up so it can travel through the same Decoded/upload path
    // instead of calling bgfx directly wherever it was found.
    static Decoded WrapRawPixels(const unsigned char* data, int w, int h, bool generateMipmaps)
    {
        Decoded out;
        if (!data || w <= 0 || h <= 0) return out;
        out.pixels.assign(data, data + (size_t)w * h * 4);
        out.width = w;
        out.height = h;
        out.generateMipmaps = generateMipmaps;
        out.valid = true;
        BuildMipsAndTransparency(out); // see DecodeFromMemoryCompressed
        return out;
    }

    // Main-thread only (does the actual bgfx::createTexture2D/updateTexture2D
    // calls) - hands already-decoded, already-mipped pixels off to
    // UploadDecodedMips().
    void UploadDecoded(Decoded&& decoded, const std::string& debugName = "")
    {
        if (!decoded.valid) return;
        // Every mip level was already computed off the main thread (see
        // Decoded::mipLevels) - this just submits the finished pixels to
        // bgfx instead of filtering them here.
        UploadDecodedMips(std::move(decoded));
        if (!debugName.empty())
            setName(debugName);
    }

    // Drops the GPU resource (and CPU mip cache) but keeps this object alive
    // at its current address - UploadDecoded() later just refills it. Main
    // thread only, same as UploadDecoded.
    void UnloadGPU()
    {
        if (bgfx::isValid(m_handle))
        {
            ResourceStatistics::Instance().unregisterResource(ResourceType::Texture, m_handle.idx);
            bgfx::destroy(m_handle);
            m_handle = BGFX_INVALID_HANDLE;
        }
        m_pixels.clear();
        m_pixels.shrink_to_fit();
        valid = false;
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
    // Builds the full mip pyramid (box-filtered, each level cascading from
    // the previous one - the same filter setupTexture_VeryFastMips used to
    // run inline) plus the transparency flag, entirely on whichever thread
    // calls it. Called from DecodeFromMemoryCompressed()/WrapRawPixels() -
    // i.e. the Loader thread for the normal async path (see
    // AssetRegistry::QueueTextureUploadJob) - so none of this per-pixel work
    // is left for UploadDecodedMips() to do on the main thread.
    static void BuildMipsAndTransparency(Decoded& out)
    {
        constexpr int bpp = 4; // Decoded is always tightly packed RGBA8
        out.mipLevels.clear();
        out.transparent = false;

        const bool hasMips = out.generateMipmaps && (out.width > 1 || out.height > 1);
        if (!hasMips)
        {
            // No mip chain - scan mip 0 directly. Still negligible in the
            // common case.
            const size_t pixelCount = (size_t)out.width * out.height;
            for (size_t i = 0; i < pixelCount; ++i) {
                if (out.pixels[i * bpp + 3] < 255) { out.transparent = true; break; }
            }
            return;
        }

        const uint8_t numMips = (uint8_t)(1 + (int)std::floor(std::log2((double)std::max(out.width, out.height))));
        out.mipLevels.reserve(numMips - 1);

        const uint8_t* srcData = out.pixels.data();
        int currentW = out.width, currentH = out.height;
        for (uint8_t mip = 1; mip < numMips; ++mip) {
            const int dstW = std::max(1, currentW / 2);
            const int dstH = std::max(1, currentH / 2);
            std::vector<uint8_t> dstMip((size_t)dstW * dstH * bpp);
            downsample2x2Box(srcData, currentW, currentH, dstMip.data(), dstW, dstH, bpp);

            out.mipLevels.push_back(std::move(dstMip));
            srcData = out.mipLevels.back().data(); // next level cascades from this one
            currentW = dstW;
            currentH = dstH;
        }

        // Detect transparency from the last (smallest) mip - a handful of
        // pixels, essentially free next to the filtering work above.
        const auto& lastMip = out.mipLevels.back();
        const size_t pixelCount = lastMip.size() / bpp;
        for (size_t i = 0; i < pixelCount; ++i) {
            if (lastMip[i * bpp + 3] < 255) { out.transparent = true; break; }
        }
    }

    // Main-thread only (the actual bgfx::createTexture2D/updateTexture2D
    // calls) - counterpart to setupTexture_VeryFastMips for the Decoded
    // pipeline. Unlike that function, every mip level here was already
    // computed by BuildMipsAndTransparency() before this Texture ever saw
    // it, so this is just a handful of cheap bgfx::copy + updateTexture2D
    // calls instead of a box filter over every pixel - that split is the
    // whole point: it's what keeps mip generation off the main thread.
    void UploadDecodedMips(Decoded&& decoded)
    {
        constexpr int bpp = 4; // Decoded is always tightly packed RGBA8
        const bool hasMips = !decoded.mipLevels.empty();
        const uint8_t numMips = (uint8_t)(1 + decoded.mipLevels.size());

        m_handle = bgfx::createTexture2D(
            (uint16_t)decoded.width, (uint16_t)decoded.height,
            hasMips, 1, bgfx::TextureFormat::RGBA8, buildFlags());
        if (!bgfx::isValid(m_handle)) {
            Logger::Error("bgfx::createTexture2D failed (%dx%d)", decoded.width, decoded.height);
            return;
        }

        // Upload mip 0.
        bgfx::updateTexture2D(m_handle, 0, 0,
            0, 0, (uint16_t)decoded.width, (uint16_t)decoded.height,
            bgfx::copy(decoded.pixels.data(), (uint32_t)decoded.pixels.size()));

        // Upload the rest - already downsampled, just hand them to bgfx.
        int currentW = decoded.width, currentH = decoded.height;
        for (uint8_t mip = 1; mip < numMips; ++mip) {
            const int dstW = std::max(1, currentW / 2);
            const int dstH = std::max(1, currentH / 2);
            const auto& mipData = decoded.mipLevels[mip - 1];
            bgfx::updateTexture2D(m_handle, 0, mip,
                0, 0, (uint16_t)dstW, (uint16_t)dstH,
                bgfx::copy(mipData.data(), (uint32_t)mipData.size()));
            currentW = dstW;
            currentH = dstH;
        }

        transparent = decoded.transparent;
        width = decoded.width;
        height = decoded.height;
        m_bpp = bpp;
        m_pixels = std::move(decoded.pixels); // CPU copy for SampleRGB()

        {
            size_t baseSize = (size_t)width * height * bpp;
            size_t trackedSize = hasMips ? (baseSize + baseSize / 3) : baseSize;
            ResourceStatistics::Instance().registerResource(
                ResourceType::Texture, m_handle.idx,
                trackedSize,
                "Texture*" + std::to_string(width) + "x" + std::to_string(height) + formatSuffix(bgfx::TextureFormat::RGBA8));
        }
        valid = true;

        // Single place this runs for the Decoded pipeline - the one
        // function that actually touches bgfx here, so the one place
        // allowed to flip currentTier to Visual.
        loadState.generation.fetch_add(1, std::memory_order_release);
        loadState.currentTier.store(AssetLoadTier::Visual, std::memory_order_release);
    }

    void setupTexture_VeryFastMips(int w, int h,
        bgfx::TextureFormat::Enum format,
        const void* pixels,
        bool generateMipmaps)
    {
        if (w <= 0 || h <= 0 || !pixels) 
        {
            Logger::Error("setupTexture: invalid arguments");
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
            Logger::Error("bgfx::createTexture2D failed");
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
        // Track the full mip chain, not just mip 0 — a generated chain adds
        // ~1/3 more memory on top of the base level (sum of a geometric
        // series with ratio 1/4 in area per level: 1 + 1/4 + 1/16 + ... = 4/3).
        // Matches the precision already used in TextureCube.hpp.
        {
            size_t baseSize = (size_t)w * h * bpp;
            size_t trackedSize = hasMips ? (baseSize + baseSize / 3) : baseSize;
            ResourceStatistics::Instance().registerResource(
                ResourceType::Texture, m_handle.idx,
                trackedSize,
                "Texture*" + std::to_string(w) + "x" + std::to_string(h) + formatSuffix(format));
        }
        valid = true;

        // Single place this runs regardless of entry path (sync ctor, raw
        // data, or UploadDecoded) - the one function that actually touches
        // bgfx, so the one place allowed to flip currentTier to Visual.
        loadState.generation.fetch_add(1, std::memory_order_release);
        loadState.currentTier.store(AssetLoadTier::Visual, std::memory_order_release);
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
            Logger::Error("setupTexture: invalid arguments (%dx%d, pixels=%s)", w, h, pixels ? "ok" : "null");
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
            Logger::Error("bgfx::createTexture2D failed (%dx%d)", base_w, base_h);
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
        {
            size_t baseSize = (size_t)w * h * bpp;
            size_t trackedSize = hasMips ? (baseSize + baseSize / 3) : baseSize;
            ResourceStatistics::Instance().registerResource(
                ResourceType::Texture, m_handle.idx,
                trackedSize,
                "Texture*" + std::to_string(w) + "x" + std::to_string(h) + formatSuffix(format));
        }
        valid = true;
    }
    // -----------------------------------------------------------------------
    // Load paths
    // -----------------------------------------------------------------------
    void loadFromFile(const std::string& filename, bool generateMipmaps) {
        UploadDecoded(DecodeFromFile(filename, generateMipmaps), filename);
    }
    void loadFromMemoryCompressed(const unsigned char* data, size_t size, bool generateMipmaps) {
        UploadDecoded(DecodeFromMemoryCompressed(data, size, generateMipmaps));
    }
    void loadFromRawData(const unsigned char* data, int w, int h,
        bgfx::TextureFormat::Enum format, bool generateMipmaps)
    {
        if (!data) {
            Logger::Error("Texture: loadFromRawData called with null data pointer");
            return;
        }
        setupTexture(w, h, format, data, generateMipmaps);
    }
};