#pragma once
#include <bgfx/bgfx.h>
#include <cstdint>


class BgfxStateManager
{
public:
    BgfxStateManager() = delete;

    enum class DepthTest : uint64_t
    {
        None = 0,
        Less = BGFX_STATE_DEPTH_TEST_LESS,
        LEqual = BGFX_STATE_DEPTH_TEST_LEQUAL,
        Equal = BGFX_STATE_DEPTH_TEST_EQUAL,
        GEqual = BGFX_STATE_DEPTH_TEST_GEQUAL,
        Greater = BGFX_STATE_DEPTH_TEST_GREATER,
        NotEqual = BGFX_STATE_DEPTH_TEST_NOTEQUAL,
        Never = BGFX_STATE_DEPTH_TEST_NEVER,
        Always = BGFX_STATE_DEPTH_TEST_ALWAYS,
    };

    enum class Cull : uint64_t
    {
        None = 0,
        CW = BGFX_STATE_CULL_CW,
        CCW = BGFX_STATE_CULL_CCW,
    };

    enum class Blend : uint64_t
    {
        None = 0,
        Alpha = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
            BGFX_STATE_BLEND_INV_SRC_ALPHA),
            Additive = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                BGFX_STATE_BLEND_ONE),
                Premultiplied = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,
                    BGFX_STATE_BLEND_INV_SRC_ALPHA),
                    Multiply = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR,
                        BGFX_STATE_BLEND_ZERO),
    };

    // -----------------------------------------------------------------------
    // Reset to a sensible default:
    //   write RGB+A, no depth test, no depth write, no cull, no blend
    // -----------------------------------------------------------------------
    static void Reset()
    {
        s_state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
    }

    // -----------------------------------------------------------------------
    // Write masks
    // -----------------------------------------------------------------------
    static void SetWriteRGB(bool enable)
    {
        SetFlag(BGFX_STATE_WRITE_RGB, enable);
    }

    static void SetWriteAlpha(bool enable)
    {
        SetFlag(BGFX_STATE_WRITE_A, enable);
    }

    static void SetWriteDepth(bool enable)
    {
        SetFlag(BGFX_STATE_WRITE_Z, enable);
    }

    // -----------------------------------------------------------------------
    // Depth test
    // -----------------------------------------------------------------------
    static void SetDepthTest(DepthTest mode)
    {
        // Clear all depth-test bits first, then apply the chosen mode
        s_state &= ~BGFX_STATE_DEPTH_TEST_MASK;
        s_state |= static_cast<uint64_t>(mode);
    }

    // -----------------------------------------------------------------------
    // Culling
    // -----------------------------------------------------------------------
    static void SetCull(Cull mode)
    {
        s_state &= ~BGFX_STATE_CULL_MASK;
        s_state |= static_cast<uint64_t>(mode);
    }

    // -----------------------------------------------------------------------
    // Blending
    // -----------------------------------------------------------------------
    static void SetBlend(Blend mode)
    {
        s_state &= ~BGFX_STATE_BLEND_MASK;
        s_state |= static_cast<uint64_t>(mode);
    }

    // -----------------------------------------------------------------------
    // MSAA
    // -----------------------------------------------------------------------
    static void SetMSAA(bool enable)
    {
        SetFlag(BGFX_STATE_MSAA, enable);
    }

    // -----------------------------------------------------------------------
    // Primitive topology
    // -----------------------------------------------------------------------
    enum class Primitive : uint64_t
    {
        TriangleList = 0, // bgfx default — no flag needed
        TriangleStrip = BGFX_STATE_PT_TRISTRIP,
        Lines = BGFX_STATE_PT_LINES,
        LineStrip = BGFX_STATE_PT_LINESTRIP,
        Points = BGFX_STATE_PT_POINTS,
    };

    static void SetPrimitive(Primitive prim)
    {
        s_state &= ~BGFX_STATE_PT_MASK;
        s_state |= static_cast<uint64_t>(prim);
    }

    // -----------------------------------------------------------------------
    // Raw flag access — escape hatch for anything not covered above
    // -----------------------------------------------------------------------
    static void AddFlags(uint64_t flags) { s_state |= flags; }
    static void RemoveFlags(uint64_t flags) { s_state &= ~flags; }
    static uint64_t GetState() { return s_state; }

    // -----------------------------------------------------------------------
    // Apply — calls bgfx::setState with the accumulated state.
    // Call this immediately before bgfx::submit().
    // -----------------------------------------------------------------------
    static void Apply()
    {
        bgfx::setState(s_state);
    }

private:
    static void SetFlag(uint64_t flag, bool enable)
    {
        if (enable) s_state |= flag;
        else        s_state &= ~flag;
    }

    static inline uint64_t s_state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
};