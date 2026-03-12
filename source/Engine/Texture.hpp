#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "FileSystem/FileSystem.h"

#include <includedLibraries/stb_image.h>
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

    bgfx::TextureHandle getHandle() const {
        return m_handle;
    }

    // Backward-compat: return numeric ID for ResourceStatistics etc.
    uint16_t getID() const 
    {

		if (bgfx::isValid(m_handle))
            return m_handle.idx;
        else
            return 0; // or some invalid ID value

        return m_handle.idx;
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
        // BGFX_TEXTURE_NONE is 0 and has no effect — omitted.
        // Anisotropic filtering for both min and mag.
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
    void setupTexture(int width, int height,
        bgfx::TextureFormat::Enum format,
        const void* pixels,
        bool generateMipmaps)
    {
        if (width <= 0 || height <= 0 || !pixels)
        {
            std::cerr << "setupTexture: invalid arguments ("
                << width << "x" << height << ", pixels="
                << (pixels ? "ok" : "null") << ")\n";
            return;
        }

        const uint32_t dataSize = (uint32_t)(width * height * bytesPerPixel(format));
        const bgfx::Memory* mem = bgfx::copy(pixels, dataSize);

        // hasMips=false: we are only supplying the base mip level.
        // Passing hasMips=true would tell bgfx that the memory buffer already
        // contains a full pre-built mip chain — it does NOT auto-generate mips.
        // Passing true with only base-level data causes bgfx to read past the
        // end of the allocation, corrupting the uniform cache and crashing on
        // the next bgfx::frame().
        //
        // To use mipmaps, pre-generate them with bimg::imageGenerateMips before
        // calling this function, or load a pre-mipped DDS/KTX file via
        // bgfx::makeRef + bgfx::createTexture.
        m_handle = bgfx::createTexture2D(
            (uint16_t)width,
            (uint16_t)height,
            false,  // hasMips — always false: we only provide the base level
            1,      // numLayers
            format,
            buildFlags(),
            mem
        );

        if (!bgfx::isValid(m_handle)) {
            std::cerr << "bgfx::createTexture2D failed (" << width << "x" << height << ")\n";
            return;
        }

        // Approximate VRAM cost (base level only — no mip chain)
        const size_t textureSize = dataSize;

        std::string textureName = "Texture_"
            + std::to_string(width) + "x" + std::to_string(height)
            + formatSuffix(format);

        ResourceStatistics::Instance().registerResource(
            ResourceType::Texture,
            m_handle.idx,
            textureSize,
            textureName
        );

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

        int width, height, channels;
        // Force RGBA8 — universally supported by bgfx across all backends
        unsigned char* pixels = stbi_load_from_memory(
            data, (int)size, &width, &height, &channels, 4);

        if (!pixels) {
            std::cerr << "Texture: stbi_load_from_memory failed: " << stbi_failure_reason() << "\n";
            return;
        }

        setupTexture(width, height, bgfx::TextureFormat::RGBA8, pixels, generateMipmaps);
        stbi_image_free(pixels);
    }

    void loadFromRawData(const unsigned char* data, int width, int height,
        bgfx::TextureFormat::Enum format, bool generateMipmaps)
    {
        if (!data) {
            std::cerr << "Texture: loadFromRawData called with null data pointer\n";
            return;
        }
        setupTexture(width, height, format, data, generateMipmaps);
    }
};