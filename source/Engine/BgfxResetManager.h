#pragma once

#include <bgfx/bgfx.h>
#include <glm.h>
#include <cstdint>
#include <Profiling/ResourceStatistics.hpp>

class BgfxResetManager
{
public:
    BgfxResetManager() = delete;

    // ---------------------------------------------------------------------
    // Reset to defaults
    // ---------------------------------------------------------------------
    static void Reset()
    {
        s_flags = BGFX_RESET_NONE;
        s_resolution = { 1280, 720 };
        s_format = bgfx::TextureFormat::RGBA8;
        s_dirty = true;
    }

    // ---------------------------------------------------------------------
    // Resolution
    // ---------------------------------------------------------------------
    static void SetResolution(const glm::uvec2& resolution)
    {
        if (s_resolution != resolution)
        {
            s_resolution = resolution;
            s_dirty = true;
        }
    }

    static void SetResolution(uint32_t width, uint32_t height)
    {
        SetResolution(glm::uvec2(width, height));
    }

    static const glm::uvec2& GetResolution()
    {
        return s_resolution;
    }

    static uint32_t GetWidth()
    {
        return s_resolution.x;
    }

    static uint32_t GetHeight()
    {
        return s_resolution.y;
    }

    // ---------------------------------------------------------------------
    // Backbuffer format
    // ---------------------------------------------------------------------
    static void SetFormat(bgfx::TextureFormat::Enum format)
    {
        if (s_format != format)
        {
            s_format = format;
            s_dirty = true;
        }
    }

    static bgfx::TextureFormat::Enum GetFormat()
    {
        return s_format;
    }

    // ---------------------------------------------------------------------
    // Generic flag handling
    // ---------------------------------------------------------------------
    static void SetFlags(uint32_t flags)
    {
        if (s_flags != flags)
        {
            s_flags = flags;
            s_dirty = true;
        }
    }

    static uint32_t GetFlags()
    {
        return s_flags;
    }

    static void AddFlags(uint32_t flags)
    {
        const uint32_t old = s_flags;
        s_flags |= flags;

        if (old != s_flags)
        {
            s_dirty = true;
        }
    }

    static void RemoveFlags(uint32_t flags)
    {
        const uint32_t old = s_flags;
        s_flags &= ~flags;

        if (old != s_flags)
        {
            s_dirty = true;
        }
    }

    static void SetFlag(uint32_t flag, bool enable)
    {
        const uint32_t old = s_flags;

        if (enable) s_flags |= flag;
        else        s_flags &= ~flag;

        if (old != s_flags)
        {
            s_dirty = true;
        }
    }

    static bool GetFlag(uint32_t flag)
    {
        return (s_flags & flag) != 0;
    }

    // ---------------------------------------------------------------------
    // Common reset flags
    // ---------------------------------------------------------------------
    static void SetFullscreen(bool enable)
    {
        SetFlag(BGFX_RESET_FULLSCREEN, enable);
    }

    static bool GetFullscreen()
    {
        return GetFlag(BGFX_RESET_FULLSCREEN);
    }

    static void SetVSync(bool enable)
    {
        SetFlag(BGFX_RESET_VSYNC, enable);
    }

    static bool GetVSync()
    {
        return GetFlag(BGFX_RESET_VSYNC);
    }

    static void SetMaxAnisotropy(bool enable)
    {
        SetFlag(BGFX_RESET_MAXANISOTROPY, enable);
    }

    static bool GetMaxAnisotropy()
    {
        return GetFlag(BGFX_RESET_MAXANISOTROPY);
    }

    static void SetCapture(bool enable)
    {
        SetFlag(BGFX_RESET_CAPTURE, enable);
    }

    static bool GetCapture()
    {
        return GetFlag(BGFX_RESET_CAPTURE);
    }

    static void SetFlushAfterRender(bool enable)
    {
        SetFlag(BGFX_RESET_FLUSH_AFTER_RENDER, enable);
    }

    static bool GetFlushAfterRender()
    {
        return GetFlag(BGFX_RESET_FLUSH_AFTER_RENDER);
    }

    static void SetFlipAfterRender(bool enable)
    {
        SetFlag(BGFX_RESET_FLIP_AFTER_RENDER, enable);
    }

    static bool GetFlipAfterRender()
    {
        return GetFlag(BGFX_RESET_FLIP_AFTER_RENDER);
    }

    static void SetSrgbBackbuffer(bool enable)
    {
        SetFlag(BGFX_RESET_SRGB_BACKBUFFER, enable);
    }

    static bool GetSrgbBackbuffer()
    {
        return GetFlag(BGFX_RESET_SRGB_BACKBUFFER);
    }

    static void SetHdr10(bool enable)
    {
        SetFlag(BGFX_RESET_HDR10, enable);
    }

    static bool GetHdr10()
    {
        return GetFlag(BGFX_RESET_HDR10);
    }

    static void SetHiDpi(bool enable)
    {
        SetFlag(BGFX_RESET_HIDPI, enable);
    }

    static bool GetHiDpi()
    {
        return GetFlag(BGFX_RESET_HIDPI);
    }

    static void SetDepthClamp(bool enable)
    {
        SetFlag(BGFX_RESET_DEPTH_CLAMP, enable);
    }

    static bool GetDepthClamp()
    {
        return GetFlag(BGFX_RESET_DEPTH_CLAMP);
    }

    static void SetSuspend(bool enable)
    {
        SetFlag(BGFX_RESET_SUSPEND, enable);
    }

    static bool GetSuspend()
    {
        return GetFlag(BGFX_RESET_SUSPEND);
    }

    static void SetTransparentBackbuffer(bool enable)
    {
        SetFlag(BGFX_RESET_TRANSPARENT_BACKBUFFER, enable);
    }

    static bool GetTransparentBackbuffer()
    {
        return GetFlag(BGFX_RESET_TRANSPARENT_BACKBUFFER);
    }

    // ---------------------------------------------------------------------
    // Apply
    // ---------------------------------------------------------------------
    static bool ApplyIfNeeded()
    {
        if (!s_dirty)
        {
            return false;
        }

        Apply();
        return true;
    }

    static void Apply()
    {
        bgfx::reset(
            s_resolution.x,
            s_resolution.y,
            s_flags,
            s_format);

        s_dirty = false;

        UpdateBackbufferTracking();
    }

    static bool IsDirty()
    {
        return s_dirty;
    }

private:
    // The backbuffer (and, on most backends, its companion depth-stencil
    // surface) is never exposed to us as a bgfx::TextureHandle -- bgfx and
    // the platform own the swap chain directly. It's still real, often
    // sizeable VRAM (especially with MSAA), so we track it here under two
    // reserved synthetic IDs, built on ResourceStatistics::kSyntheticIdBase
    // so it's excluded from any comparison against bgfx's own internal
    // counters (which structurally can never include the backbuffer) while
    // still counting toward Total Memory and driver-reported comparisons.
    //
    // NOTE: this is a best-effort estimate, not an exact figure -- bgfx
    // doesn't expose the true backbuffer allocation size through its public
    // API, and not every backend actually allocates a depth-stencil surface
    // this way. Cross-check against ResourceStatistics's driver-reported
    // line (bgfx::getStats()->gpuMemoryUsed) for the real ground truth.
    static constexpr uint64_t kBackbufferColorId = ResourceStatistics::kSyntheticIdBase;
    static constexpr uint64_t kBackbufferDepthId = ResourceStatistics::kSyntheticIdBase + 1;

    static uint32_t BytesPerPixelForFormat(bgfx::TextureFormat::Enum format)
    {
        switch (format)
        {
        case bgfx::TextureFormat::RGBA16:
        case bgfx::TextureFormat::RGBA16F:
            return 8;
        case bgfx::TextureFormat::RGBA32F:
            return 16;
        case bgfx::TextureFormat::RGBA8:
        case bgfx::TextureFormat::BGRA8:
        case bgfx::TextureFormat::RGB10A2:
        default:
            return 4; // best-effort default for uncommon backbuffer formats
        }
    }

    static uint32_t MsaaSampleCount(uint32_t flags)
    {
        switch (flags & BGFX_RESET_MSAA_MASK)
        {
        case BGFX_RESET_MSAA_X2:  return 2;
        case BGFX_RESET_MSAA_X4:  return 4;
        case BGFX_RESET_MSAA_X8:  return 8;
        case BGFX_RESET_MSAA_X16: return 16;
        default:                  return 1;
        }
    }

    static void UpdateBackbufferTracking()
    {
        const uint32_t samples = MsaaSampleCount(s_flags);
        const size_t colorBytes =
            (size_t)s_resolution.x * s_resolution.y * BytesPerPixelForFormat(s_format) * samples;
        // Most bgfx backends auto-allocate a matching D24S8 depth-stencil
        // surface (4 bytes/pixel) for the default back buffer.
        const size_t depthBytes =
            (size_t)s_resolution.x * s_resolution.y * 4 * samples;

        ResourceStatistics::Instance().registerResource(
            ResourceType::RenderTexture, kBackbufferColorId, colorBytes,
            "Backbuffer Color (approx.)");
        ResourceStatistics::Instance().registerResource(
            ResourceType::RenderTexture, kBackbufferDepthId, depthBytes,
            "Backbuffer Depth/Stencil (approx.)");
    }

    static inline glm::uvec2 s_resolution = { 1280, 720 };
    static inline bgfx::TextureFormat::Enum s_format = bgfx::TextureFormat::RGBA8;
    static inline uint32_t s_flags = BGFX_RESET_NONE;
    static inline bool s_dirty = true;
};