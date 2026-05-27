#pragma once

#include <bgfx/bgfx.h>
#include <glm.h>
#include <cstdint>

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
        s_format = bgfx::TextureFormat::Count;
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
    }

    static bool IsDirty()
    {
        return s_dirty;
    }

private:
    static inline glm::uvec2 s_resolution = { 1280, 720 };
    static inline bgfx::TextureFormat::Enum s_format = bgfx::TextureFormat::Count;
    static inline uint32_t s_flags = BGFX_RESET_NONE;
    static inline bool s_dirty = true;
};