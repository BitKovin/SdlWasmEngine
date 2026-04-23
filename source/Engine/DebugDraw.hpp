#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <cstdint>
#include <bgfx/bgfx.h>

#include "glm.h"
#include "Delay.hpp"

// ─── Color constants (ABGR – bgfx vertex attribute convention) ────────────────
//
//   When stored as a uint32_t on a little-endian machine the byte layout is:
//     byte[0] = R,  byte[1] = G,  byte[2] = B,  byte[3] = A
//   which is the same as writing the literal 0xAABBGGRR.
//
namespace DebugColor
{
    constexpr uint32_t White   = 0xFFFFFFFF;
    constexpr uint32_t Black   = 0xFF000000;
    constexpr uint32_t Red     = 0xFF0000FF;
    constexpr uint32_t Green   = 0xFF00FF00;
    constexpr uint32_t Blue    = 0xFFFF0000;
    constexpr uint32_t Yellow  = 0xFF00FFFF;
    constexpr uint32_t Cyan    = 0xFFFFFF00;
    constexpr uint32_t Magenta = 0xFFFF00FF;
    constexpr uint32_t Orange  = 0xFF0080FF;
    constexpr uint32_t Gray    = 0xFF808080;

    /// Build a color from individual RGBA bytes (all 0-255).
    inline constexpr uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return (static_cast<uint32_t>(a) << 24)
             | (static_cast<uint32_t>(b) << 16)
             | (static_cast<uint32_t>(g) <<  8)
             |  static_cast<uint32_t>(r);
    }
}

// ─── Internal timed-line command ─────────────────────────────────────────────

struct DebugLineCommand
{
    vec3     start;
    vec3     end;
    uint32_t color;   ///< ABGR packed
    Delay    drawTime;

    DebugLineCommand(vec3 s, vec3 e, uint32_t col, double duration)
        : start(s), end(e), color(col), drawTime(duration, true) {}
};




// ─── DebugDraw ────────────────────────────────────────────────────────────────
//
//  Thread model
//  ─────────────
//  Any thread may call the shape helpers (Line / Bounds / Path / IndexedMesh).
//  The game/simulation thread calls Finalize() once per frame to prune expired
//  commands and snapshot raw pointers.
//  The render thread calls Draw() after setting the view transform:
//
//      bgfx::setViewTransform(viewId, &view[0][0], &proj[0][0]);
//      DebugDraw::Draw();
//
//  All debug geometry is submitted in a single bgfx draw call using a transient
//  vertex buffer – no per-line mesh objects, no allocations at draw time.
//
//  Thickness
//  ─────────
//  The 'thickness' parameter is preserved for API compatibility but is not
//  applied.  bgfx line primitives are always 1 px wide.  If you need thick
//  lines you can replace the LINE primitive with billboard quads; that is left
//  as a future exercise.
//
class DebugDraw
{
public:
    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// One-time initialisation.  Call after bgfx::init().
    /// 'view' is the bgfx view ID that debug geometry will be submitted to.
    static void Init();

    /// Release all bgfx resources.  Call before bgfx::shutdown().
    static void Shutdown();

    /// Immediately discard all pending draw commands.
    static void ClearCommands();

    // ── Shape helpers ─────────────────────────────────────────────────────

    static void Line(vec3     start,
                     vec3     end,
                     float    duration  = 0.1f,
                     float    thickness = 0.02f,
                     uint32_t color     = DebugColor::Red);

    static void Bounds(vec3     min,
                       vec3     max,
                       float    duration  = 0.1f,
                       float    thickness = 0.02f,
                       uint32_t color     = DebugColor::Red);

    static void Path(std::vector<vec3> path,
                     float    duration  = 1.0f,
                     float    thickness = 0.02f,
                     uint32_t color     = DebugColor::Red);

    static void IndexedMesh(std::vector<vec3>     vertices,
                            std::vector<uint32_t> indices,
                            float    duration  = 10.0f,
                            float    thickness = 0.02f,
                            uint32_t color     = DebugColor::Red);

    // ── Frame lifecycle ───────────────────────────────────────────────────

    /// Prune expired commands; snapshot raw pointers for the render thread.
    /// Call once per frame from the game/simulation thread before Draw().
    static void Finalize();

    /// Upload a transient vertex buffer and submit one draw call.
    /// The caller must have already called:
    ///   bgfx::setViewTransform(viewId, viewMtx, projMtx);
    static void Draw();

private:
    // Packed vertex: position + ABGR color.
    struct DebugVertex
    {
        float    x, y, z;
        uint32_t abgr;

        static bgfx::VertexLayout layout;
        static void init();
    };

    static std::mutex mainLock;
    static std::vector<std::unique_ptr<DebugLineCommand>> commands;
    static std::vector<DebugLineCommand*>                  finalizedCommands;

    static bgfx::ProgramHandle program;
    static bgfx::ViewId        viewId;
    static bool                initialized;

    static bgfx::DynamicVertexBufferHandle vertexBuffer;
    static uint32_t                        vertexCapacity;


    static void EnsureVertexBuffer(uint32_t requiredVertices)
    {
        if (bgfx::isValid(DebugDraw::vertexBuffer) &&
            requiredVertices <= DebugDraw::vertexCapacity)
            return;

        if (bgfx::isValid(DebugDraw::vertexBuffer))
        {
            bgfx::destroy(DebugDraw::vertexBuffer);
            DebugDraw::vertexBuffer = BGFX_INVALID_HANDLE;
        }

        DebugDraw::vertexCapacity = std::max(requiredVertices, 256u);

        DebugDraw::vertexBuffer = bgfx::createDynamicVertexBuffer(
            DebugDraw::vertexCapacity,
            DebugDraw::DebugVertex::layout,
            BGFX_BUFFER_ALLOW_RESIZE);
    }

};
