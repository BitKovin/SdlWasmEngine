#include "UiRenderer.h"
#include <bgfx/bgfx.h>
#include "../ShaderManager.h"
#include "../Camera.h"
#include <unordered_map>
#include <SDL2/SDL.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include "../Time.hpp"
#include <mutex>
#include "UiManager.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

#include <includedLibraries/stb_truetype.h>

// ─────────────────────────────────────────────────────────────────────────────
// Quad vertex layout & static GPU resources
// ─────────────────────────────────────────────────────────────────────────────

struct QuadVertex {
    float x, y; // position (screen-space or model-space)
    float u, v; // texcoord
};

static bgfx::VertexLayout       s_quadLayout;
static bgfx::VertexBufferHandle s_quadVB = BGFX_INVALID_HANDLE;
static Shader* s_texturedShader = nullptr;
static Shader* s_flatColorShader = nullptr;
static float   currentTime = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
// Per-codepoint glyph record stored in a FontAtlas
// ─────────────────────────────────────────────────────────────────────────────

struct GlyphInfo {
    // Atlas UV coordinates (normalized 0..1)
    float u0 = 0.f, v0 = 0.f;
    float u1 = 0.f, v1 = 0.f;
    // Glyph bitmap dimensions (pixels)
    int bitmapW = 0, bitmapH = 0;
    // Bearing: offset from pen origin to top-left of bitmap (pixels)
    int   xoff = 0;
    int   yoff = 0;
    // Horizontal advance (pixels, already scaled)
    float advanceX = 0.f;
    // True when the glyph has no visible pixels (space, control chars)
    bool  invisible = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// FontAtlas
//   - Owns the raw .ttf data and stbtt_fontinfo.
//   - Maintains a CPU-side RGBA8 bitmap (white pixels, alpha = coverage).
//   - Glyphs are packed left-to-right / top-to-bottom on demand.
//   - Uploads to a bgfx texture once per frame when dirty.
//
//   THREADING CONTRACT:
//   - `mutex` guards the CPU-side packing state: packX/packY/rowH, pixels,
//     glyphs, and the dirty rect. Any caller of EnsureGlyph() or
//     FlushToGPU() must hold `mutex` for the full duration of the call —
//     see DrawText() and MeasureText() for the pattern. This protects
//     against e.g. MeasureText (possibly called from a layout/worker
//     thread) racing DrawText (render thread) on the same atlas's glyph
//     cache. Do not lock inside EnsureGlyph/FlushToGPU themselves, or
//     callers that already hold the lock will deadlock.
//   - `texture` (the bgfx handle) and CreateGpuResources() are NOT
//     protected by `mutex`, and don't need to be, on one condition: every
//     bgfx call this atlas ever makes (CreateGpuResources, FlushToGPU,
//     Destroy) must happen on the same single thread — whichever thread
//     drives DrawText/EndFrame/bgfx::frame. bgfx does not support being
//     called concurrently from multiple threads outside of its own
//     multithreading API, so this is a hard requirement, not a suggestion.
//     LoadCpuData() is the one exception: it makes no bgfx calls at all,
//     so it's safe to run on a background loading thread.
// ─────────────────────────────────────────────────────────────────────────────

struct FontAtlas {
    // ── stb_truetype state ────────────────────────────────────────────────────
    stbtt_fontinfo       fontInfo{};
    std::vector<uint8_t> fileData;       // raw .ttf bytes; must outlive fontInfo
    float  scale = 1.0f;          // stbtt_ScaleForPixelHeight result
    float  pixelHeight = 16.0f;
    int    ascent = 0;
    int    descent = 0;
    int    lineGap = 0;

    // ── Atlas bitmap ─────────────────────────────────────────────────────────
    static constexpr int ATLAS_W = 2048 * 2;
    static constexpr int ATLAS_H = 2048;
    std::vector<uint8_t> pixels;         // ATLAS_W * ATLAS_H * 4 (RGBA8)
    bgfx::TextureHandle  texture = BGFX_INVALID_HANDLE;
    bool textureDirty = false;

    // ── Packing & Padding configuration ──────────────────────────────────────
    int padding = 10;
    int packX = 2; // Initialized dynamically in LoadCpuData() based on padding
    int packY = 2;
    int rowH = 0; // tallest glyph in the current row

    // ── Glyph cache ───────────────────────────────────────────────────────────
    std::unordered_map<int, GlyphInfo> glyphs;

    int dirtyX0 = ATLAS_W, dirtyY0 = ATLAS_H;
    int dirtyX1 = 0, dirtyY1 = 0;

    // ── Concurrency ───────────────────────────────────────────────────────────
    // Guards packX/packY/rowH, pixels, glyphs, and the dirty rect above.
    // See the class-level comment for the full locking contract.
    std::mutex mutex;

    void MarkDirty(int x0, int y0, int x1, int y1)
    {
        dirtyX0 = std::min(dirtyX0, x0);
        dirtyY0 = std::min(dirtyY0, y0);
        dirtyX1 = std::max(dirtyX1, x1);
        dirtyY1 = std::max(dirtyY1, y1);
        textureDirty = true;
    }


    // ── Load / Init / Destroy ────────────────────────────────────────────────

    // CPU-only: reads the .ttf file, parses it with stb_truetype, computes
    // metrics, and allocates + clears the CPU-side pixel buffer. Makes NO
    // bgfx calls, so this is safe to call from any thread — including a
    // background font-loading thread running concurrently with DrawText on
    // the render thread for a different (already-loaded) font atlas.
    bool LoadCpuData(const char* path, float height, int paddingSize = 3)
    {
        // Packing gap between glyphs, in atlas texels. This used to scale with
        // font height (height/3 + paddingSize) on the assumption that it also
        // had to reserve enough room for whatever shadow/outline/glow radius
        // might later be requested against this text. It no longer does:
        // DrawText's effect geometry/UV now extend past this gap freely, sized
        // from the actual viewport-pixel effect radius requested per draw (see
        // DrawText) rather than from whatever got baked in here at load time.
        // fs_effects.sc's per-glyph u_ClampRect is what actually stops that
        // from sampling into a neighboring glyph, independent of this gap's
        // size — see the clampPadX/Y comment in DrawText. So this only needs
        // to be big enough to keep bilinear texture filtering from blending
        // across the glyph boundary at zero distance.
        padding = paddingSize;
        packX = padding;
        packY = padding;
        rowH = 0;

        fileData = FileSystemEngine::ReadFileBinary(std::string(path));

        if (fileData.size() == 0)
        {
            std::cerr << "[UiRenderer] LoadFont: cannot open '" << path << "'\n";
            return false;
        }

        if (!stbtt_InitFont(&fontInfo, fileData.data(), 0)) {
            std::cerr << "[UiRenderer] LoadFont: stbtt_InitFont failed for '" << path << "'\n";
            fileData.clear();
            return false;
        }

        pixelHeight = height;

        // Fetch metrics FIRST so we can use them to define the exact scale
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        // Calculate scale manually: The exact total line height maps perfectly to requested height.
        scale = height / static_cast<float>(ascent - descent + lineGap);

        // White-transparent background
        const size_t atlasPixels = static_cast<size_t>(ATLAS_W) * ATLAS_H;
        pixels.resize(atlasPixels * 4);
        for (size_t i = 0; i < atlasPixels; ++i)
        {
            pixels[i * 4 + 0] = 255u;
            pixels[i * 4 + 1] = 255u;
            pixels[i * 4 + 2] = 255u;
            pixels[i * 4 + 3] = 0u;
        }

        textureDirty = false;
        return true;
    }

    // GPU-only: creates the bgfx texture backing this atlas. MUST be called
    // from the single thread that drives DrawText / EndFrame / bgfx::frame
    // — never from a background loading thread. Idempotent: no-ops if a
    // valid texture already exists, so it's safe to call defensively before
    // every draw.
    bool CreateGpuResources()
    {
        if (bgfx::isValid(texture))
            return true;

        // No mipmaps. bgfx default sampler = bilinear filtering + clamp.
        texture = bgfx::createTexture2D(
            static_cast<uint16_t>(ATLAS_W),
            static_cast<uint16_t>(ATLAS_H),
            false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
            nullptr);

        return bgfx::isValid(texture);
    }

    // Convenience wrapper for a caller that's fine loading synchronously on
    // the render thread and doesn't need the CPU/GPU split. Do NOT call
    // this from a background loading thread — see CreateGpuResources().
    bool Init(const char* path, float height, int paddingSize = 3)
    {
        return LoadCpuData(path, height, paddingSize) && CreateGpuResources();
    }

    void Destroy()
    {
        // Must run on the same thread as other bgfx calls for this atlas.
        std::lock_guard<std::mutex> lock(mutex);
        if (bgfx::isValid(texture)) {
            bgfx::destroy(texture);
            texture = BGFX_INVALID_HANDLE;
        }
        fileData.clear();
        pixels.clear();
        glyphs.clear();
    }

    // ── EnsureGlyph ───────────────────────────────────────────────────────────
    // CALLER MUST HOLD `mutex` before calling this — see DrawText/MeasureText.

    bool EnsureGlyph(int codepoint)
    {
        if (glyphs.count(codepoint))
            return true;

        // Rasterize the glyph into a temporary 1-channel bitmap
        int w = 0, h = 0, xoff = 0, yoff = 0;
        uint8_t* bm = stbtt_GetCodepointBitmap(
            &fontInfo, scale, scale, codepoint, &w, &h, &xoff, &yoff);

        if (!bm || w <= 0 || h <= 0) {
            // Invisible / missing glyph (e.g. space, tab) – record metrics only
            int advW = 0, lsb = 0;
            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advW, &lsb);
            GlyphInfo g{};
            g.advanceX = static_cast<float>(advW) * scale;
            g.invisible = true;
            glyphs[codepoint] = g;
            if (bm) stbtt_FreeBitmap(bm, nullptr);
            return true;
        }

        // FIX: glyph is wider than a full fresh row could ever hold, no
        // matter which row we place it on. Bail out with correct advance
        // metrics recorded instead of letting the row-wrap logic below
        // "succeed" into a row it still doesn't fit in and corrupt
        // neighboring atlas memory.
        if (w + 2 * padding > ATLAS_W) {
            std::cerr << "[UiRenderer] Font atlas: codepoint " << codepoint
                << " is wider than the atlas row capacity and cannot be packed.\n";
            stbtt_FreeBitmap(bm, nullptr);
            int advW = 0, lsb = 0;
            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advW, &lsb);
            GlyphInfo g{};
            g.advanceX = static_cast<float>(advW) * scale;
            g.invisible = true;
            glyphs[codepoint] = g;
            return false;
        }

        // Start a new row if the glyph doesn't fit horizontally (accounting for padding)
        if (packX + w + padding > ATLAS_W) {
            packX = padding;
            packY += rowH + padding;
            rowH = 0;
        }

        // Atlas exhausted – accounting for bottom padding boundary
        if (packY + h + padding > ATLAS_H) {
            std::cerr << "[UiRenderer] Font atlas full – codepoint "
                << codepoint << " will not render.\n";
            stbtt_FreeBitmap(bm, nullptr);
            // FIX: previously this left advanceX at its default of 0.f, so
            // once the atlas filled up, every subsequent new codepoint
            // stacked at the same pen position forever (and stayed that
            // way, since this result is cached permanently). Fetch the
            // real advance so layout stays correct even for glyphs that
            // can't be rasterized.
            int advW = 0, lsb = 0;
            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advW, &lsb);
            GlyphInfo g{};
            g.advanceX = static_cast<float>(advW) * scale;
            g.invisible = true;
            glyphs[codepoint] = g;
            return false;
        }

        // Blit grayscale coverage into the RGBA atlas
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                const uint8_t alpha = bm[row * w + col];
                const int idx = ((packY + row) * ATLAS_W + (packX + col)) * 4;
                pixels[idx + 0] = 255u;
                pixels[idx + 1] = 255u;
                pixels[idx + 2] = 255u;
                pixels[idx + 3] = alpha;
            }
        }

        MarkDirty(packX, packY, packX + w, packY + h);

        // Record glyph metadata
        GlyphInfo g;
        g.u0 = static_cast<float>(packX) / ATLAS_W;
        g.v0 = static_cast<float>(packY) / ATLAS_H;
        g.u1 = static_cast<float>(packX + w) / ATLAS_W;
        g.v1 = static_cast<float>(packY + h) / ATLAS_H;
        g.bitmapW = w;
        g.bitmapH = h;
        g.xoff = xoff;
        g.yoff = yoff;
        g.invisible = false;

        int advW = 0, lsb = 0;
        stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advW, &lsb);
        g.advanceX = static_cast<float>(advW) * scale;

        glyphs[codepoint] = g;

        // Advance packing cursor by the width of the glyph plus the padding space
        packX += w + padding;
        if (h > rowH) rowH = h;

        stbtt_FreeBitmap(bm, nullptr);
        textureDirty = true;
        return true;
    }

    // CALLER MUST HOLD `mutex` before calling this (same contract as
    // EnsureGlyph — it reads `pixels` and the dirty rect). Also must run
    // on the single bgfx-owning thread, since it calls bgfx::updateTexture2D.
    void FlushToGPU()
    {
        if (!textureDirty || !bgfx::isValid(texture))
            return;

        const int x = dirtyX0, y = dirtyY0;
        const int w = dirtyX1 - dirtyX0;
        const int h = dirtyY1 - dirtyY0;
        if (w <= 0 || h <= 0) { textureDirty = false; return; }

        // bgfx::copy needs contiguous memory, but our atlas rows are strided
        // by ATLAS_W, so pack just the dirty rect into a small temp buffer.
        std::vector<uint8_t> region(static_cast<size_t>(w) * h * 4);
        for (int row = 0; row < h; ++row) {
            const uint8_t* src = &pixels[((y + row) * ATLAS_W + x) * 4];
            std::memcpy(&region[row * w * 4], src, static_cast<size_t>(w) * 4);
        }

        bgfx::updateTexture2D(texture, 0, 0,
            static_cast<uint16_t>(x), static_cast<uint16_t>(y),
            static_cast<uint16_t>(w), static_cast<uint16_t>(h),
            bgfx::copy(region.data(), static_cast<uint32_t>(region.size())));

        dirtyX0 = ATLAS_W; dirtyY0 = ATLAS_H;
        dirtyX1 = 0;       dirtyY1 = 0;
        textureDirty = false;
    }

    // ── Line metrics helpers ──────────────────────────────────────────────────

    float LineHeight()  const { return static_cast<float>(ascent - descent + lineGap) * scale; }
    float BaselineOff() const { return static_cast<float>(ascent) * scale; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Font registry
// ─────────────────────────────────────────────────────────────────────────────

static std::unordered_map<uint32_t, FontAtlas*> s_fontRegistry;
static uint32_t                                  s_nextFontId = 1; // 0 == INVALID_FONT
// Guards s_fontRegistry / s_fontKeyCache ONLY. It does not, and never did,
// serialize the bgfx calls made through a FontAtlas* handed out of the
// registry — that's what FontAtlas::mutex plus the "GPU calls happen on
// one thread only" contract are for. See the class-level comment on
// FontAtlas for the full picture.
static std::mutex                                s_fontMutex;

// Cache key: path + '@' + pixel-height. Same .ttf at the same size returns the
// existing handle without allocating a second atlas.
static std::unordered_map<std::string, UiRenderer::FontHandle> s_fontKeyCache;

static std::string MakeFontKey(const char* path, float pixelHeight)
{
    return std::string(path) + "@" + std::to_string(pixelHeight);
}

// ─────────────────────────────────────────────────────────────────────────────
// UTF-8 decoder: advances *p past the current codepoint and returns it.
// Returns -1 on invalid/end-of-string.
// ─────────────────────────────────────────────────────────────────────────────

static int NextCodepoint(const char*& p)
{
    if (!*p) return -1;

    const auto u = reinterpret_cast<const unsigned char*>(p);
    int cp;

    if ((u[0] & 0x80u) == 0u) {                              // 0xxxxxxx
        cp = u[0];
        p += 1;
    }
    else if ((u[0] & 0xE0u) == 0xC0u && (u[1] & 0xC0u) == 0x80u) {  // 110xxxxx
        cp = ((u[0] & 0x1Fu) << 6) | (u[1] & 0x3Fu);
        p += 2;
    }
    else if ((u[0] & 0xF0u) == 0xE0u &&
        (u[1] & 0xC0u) == 0x80u && (u[2] & 0xC0u) == 0x80u) {   // 1110xxxx
        cp = ((u[0] & 0x0Fu) << 12) | ((u[1] & 0x3Fu) << 6) | (u[2] & 0x3Fu);
        p += 3;
    }
    else if ((u[0] & 0xF8u) == 0xF0u &&
        (u[1] & 0xC0u) == 0x80u && (u[2] & 0xC0u) == 0x80u &&
        (u[3] & 0xC0u) == 0x80u) {                               // 11110xxx
        cp = ((u[0] & 0x07u) << 18) | ((u[1] & 0x3Fu) << 12) |
            ((u[2] & 0x3Fu) << 6) | (u[3] & 0x3Fu);
        p += 4;
    }
    else {
        // Invalid byte – skip it
        cp = 0xFFFD;
        p += 1;
    }

    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
namespace UiRenderer {

    // ── Init ──────────────────────────────────────────────────────────────────────

    void Init()
    {
        // Vertex layout: float2 position + float2 texcoord
        s_quadLayout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();

        // Unit quad [0,1]×[0,1], two CCW triangles, y-down origin
        static const QuadVertex quadVertices[6] = {
            { 0.0f, 1.0f,  0.0f, 1.0f },
            { 1.0f, 0.0f,  1.0f, 0.0f },
            { 0.0f, 0.0f,  0.0f, 0.0f },
            { 0.0f, 1.0f,  0.0f, 1.0f },
            { 1.0f, 1.0f,  1.0f, 1.0f },
            { 1.0f, 0.0f,  1.0f, 0.0f },
        };

        s_quadVB = bgfx::createVertexBuffer(
            bgfx::makeRef(quadVertices, sizeof(quadVertices)),
            s_quadLayout);

        s_texturedShader = ShaderManager::GetShaderProgram("vs_ui", "fs_ui_textured");
        s_flatColorShader = ShaderManager::GetShaderProgram("vs_ui", "fs_ui_flatcolor");
    }

    // ── Shutdown ──────────────────────────────────────────────────────────────────

    void Shutdown()
    {
        if (bgfx::isValid(s_quadVB)) {
            bgfx::destroy(s_quadVB);
            s_quadVB = BGFX_INVALID_HANDLE;
        }

        s_texturedShader = nullptr;
        s_flatColorShader = nullptr;

        std::lock_guard<std::mutex> lock(s_fontMutex);
        for (auto& [id, atlas] : s_fontRegistry) {
            atlas->Destroy();
            delete atlas;
        }
        s_fontRegistry.clear();
        s_fontKeyCache.clear();
    }


    // ── LoadFont ──────────────────────────────────────────────────────────────────

    FontHandle LoadFont(const char* path, float pixelHeight)
    {
        const std::string key = MakeFontKey(path, pixelHeight);

        std::lock_guard<std::mutex> lock(s_fontMutex);

        // Return the existing handle if this path+size was already loaded
        auto cacheIt = s_fontKeyCache.find(key);
        if (cacheIt != s_fontKeyCache.end())
            return cacheIt->second;

        auto* atlas = new FontAtlas();

        // FIX: only load CPU-side data here. This used to call atlas->Init(),
        // which also called bgfx::createTexture2D — meaning whatever thread
        // invoked LoadFont() made a bgfx call. If that thread wasn't the one
        // driving DrawText/EndFrame (e.g. a background font-loading thread),
        // that bgfx::createTexture2D call could run at the exact same moment
        // as DrawText's bgfx::submit/updateTexture2D for a *different*,
        // already-loaded font — two threads hitting bgfx's API concurrently,
        // with nothing in this file serializing them (s_fontMutex only ever
        // protected s_fontRegistry/s_fontKeyCache, and DrawText releases it
        // long before making any bgfx calls). That's a very plausible source
        // of intermittent, timing-dependent glyph/texture corruption.
        //
        // GPU texture creation is now deferred to DrawText (see there),
        // lazily, the first time this atlas is actually drawn — which is
        // guaranteed to happen on a single thread. That means this function
        // now makes zero bgfx calls and is safe to call from any thread,
        // including a background loader.
        if (!atlas->LoadCpuData(path, pixelHeight)) {
            delete atlas;
            return INVALID_FONT;
        }

        const FontHandle id = s_nextFontId++;
        s_fontRegistry[id] = atlas;
        s_fontKeyCache[key] = id;
        return id;
    }

    // ── UnloadFont ────────────────────────────────────────────────────────────────
    // NOTE: only call this once no other thread might still be mid-DrawText/
    // MeasureText for this handle — it deletes the FontAtlas outright, and
    // nothing here protects against a dangling FontAtlas* held by another
    // in-flight call. That's a lifetime/ownership concern distinct from the
    // packing-state race this file otherwise guards against.

    void UnloadFont(FontHandle handle)
    {
        if (handle == INVALID_FONT) return;

        std::lock_guard<std::mutex> lock(s_fontMutex);
        auto it = s_fontRegistry.find(handle);
        if (it == s_fontRegistry.end()) return;

        it->second->Destroy();
        delete it->second;
        s_fontRegistry.erase(it);

        // Remove from key cache so the path can be reloaded fresh if needed
        for (auto kit = s_fontKeyCache.begin(); kit != s_fontKeyCache.end(); ++kit) {
            if (kit->second == handle) {
                s_fontKeyCache.erase(kit);
                break;
            }
        }
    }

    // ── Projection helper ─────────────────────────────────────────────────────────

    static void SetShaderProjection(Shader* shader)
    {
        float screenHeight = static_cast<float>(UiManager::GetScaledUiHeight());
        float screenWidth = screenHeight * Camera::AspectRatio;

        if (customViewport) {
            screenWidth = static_cast<float>(customViewportSize.x);
            screenHeight = static_cast<float>(customViewportSize.y);
        }

        const glm::mat4 uiProjection = glm::ortho(
            0.0f, screenWidth,
            screenHeight, 0.0f,
            -1.0f, 1.0f);

        shader->SetUniform("u_Projection", uiProjection);

        // Viewport pixel dimensions of whatever's actually being rendered to right
        // now — the real screen, or the custom off-screen target when UI is being
        // rendered onto a 3D billboard (see UiRenderer::customViewport). Every
        // element size/position is already expressed 1:1 in these units (see the
        // "1 UI unit == 1 output pixel" note at the top of UiRenderer.h), so this
        // is here purely so a shader can reason in absolute viewport-pixel terms
        // if it needs to. Set unconditionally (not just for effects shaders) since
        // it's cheap and correct for every draw.
        shader->SetUniform("u_ViewportSize", glm::vec4(screenWidth, screenHeight, 0.f, 0.f));
    }

    static glm::mat4 BuildQuadModel(const glm::vec2& pos, const glm::vec2& size,
        float rotation, glm::vec2 pivot)
    {
        const glm::vec2 pivotOffset = pivot * size;
        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(pos, 0.0f));
        m = glm::translate(m, glm::vec3(pivotOffset, 0.0f));
        m = glm::rotate(m, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
        m = glm::translate(m, glm::vec3(-pivotOffset, 0.0f));
        m = glm::scale(m, glm::vec3(size, 1.0f));
        return m;
    }

    // GLM is column-major: m[col][row].
    static glm::mat4 BuildQuadModelFromMat3(const glm::mat3& t, const glm::vec2& size)
    {
        glm::mat4 m(1.f);
        m[0][0] = t[0][0]; m[0][1] = t[0][1];
        m[1][0] = t[1][0]; m[1][1] = t[1][1];
        m[3][0] = t[2][0]; m[3][1] = t[2][1];
        return m * glm::scale(glm::mat4(1.f), glm::vec3(size, 1.f));
    }

    struct MaskEntry { glm::mat4 model; };
    static std::vector<MaskEntry> s_maskStack;

    static void DrawStencilRect_Internal(const glm::mat4& model, uint8_t ref)
    {
        s_flatColorShader->UseProgram();
        SetShaderProjection(s_flatColorShader);

        s_flatColorShader->SetUniform("u_Model", model);
        s_flatColorShader->SetUniform("u_Color", glm::vec4(0.f));

        // WRITE_RGB satisfies bgfx's requirement to process fragments so stencil
        // ops fire. The blend equation src*0 + dst*1 = dst preserves the colour
        // buffer entirely — neither PushMask nor PopMask taint rendered pixels.
        bgfx::setState(
            BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
            BGFX_STATE_DEPTH_TEST_ALWAYS |
            BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE)
        );

        bgfx::setStencil(
            BGFX_STENCIL_TEST_ALWAYS |
            BGFX_STENCIL_FUNC_REF(ref) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_REPLACE |
            BGFX_STENCIL_OP_FAIL_Z_REPLACE |
            BGFX_STENCIL_OP_PASS_Z_REPLACE
        );

        bgfx::setVertexBuffer(0, s_quadVB);
        s_flatColorShader->Submit(ViewIdManager::GetCurrentId());
    }

    // ── Internal: apply the active stencil test before a normal draw call ─────────
    // Called by SubmitQuad and DrawText's submit path.

    static void ApplyStencilTest()
    {
        if (s_maskStack.empty()) return;

        const uint8_t ref = static_cast<uint8_t>(s_maskStack.size());
        bgfx::setStencil(
            BGFX_STENCIL_TEST_EQUAL |  // only draw where stencil == ref
            BGFX_STENCIL_FUNC_REF(ref) |
            BGFX_STENCIL_FUNC_RMASK(0xFF) |
            BGFX_STENCIL_OP_FAIL_S_KEEP |  // stencil fail → leave stencil alone
            BGFX_STENCIL_OP_FAIL_Z_KEEP |
            BGFX_STENCIL_OP_PASS_Z_KEEP          // pass         → leave stencil alone
        );
    }

    // ── Shared submit: bind the unit quad VB and dispatch ────────────────────────

    static void SubmitQuad(Shader* shader)
    {
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
        BgfxStateManager::Apply();

        ApplyStencilTest();

        bgfx::setVertexBuffer(0, s_quadVB);
        shader->Submit(ViewIdManager::GetCurrentId());
    }

    // ── DrawTexturedRect ──────────────────────────────────────────────────────────

    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size,
        float rotation, vec2 pivot,
        bgfx::TextureHandle texture, const glm::vec4& color)
    {
        s_texturedShader->UseProgram();
        SetShaderProjection(s_texturedShader);

        s_texturedShader->SetUniform("u_Model", BuildQuadModel(pos, size, rotation, pivot));
        s_texturedShader->SetUniform("u_Color", color);
        s_texturedShader->SetTexture("u_Texture", texture);
        s_texturedShader->SetUniform("bilboard", customViewport);

        SubmitQuad(s_texturedShader);
    }

    // ── DrawTexturedRectShader ────────────────────────────────────────────────────

    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size,
        float rotation, glm::vec2 pivot,
        bgfx::TextureHandle texture, const glm::vec4& color,
        const string& shader)
    {
        auto* sp = ShaderManager::GetShaderProgram("ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);

        sp->SetUniform("u_Model", BuildQuadModel(pos, size, rotation, pivot));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);
        sp->SetUniform("u_TextureSize", size);

        SubmitQuad(sp);
    }

    // ── DrawTexturedRectShaderParams ──────────────────────────────────────────────

    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size,
        float rotation, glm::vec2 pivot,
        std::unordered_map<std::string, bgfx::TextureHandle>& textures,
        std::unordered_map<std::string, vec4>& vec4s,
        const glm::vec4& color, const string& shader)
    {
        auto* sp = ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);

        sp->SetUniform("u_Model", BuildQuadModel(pos, size, rotation, pivot));
        sp->SetUniform("u_Color", color);

        for (auto& [name, tex] : textures) sp->SetTexture(name, tex);
        for (auto& [name, v4] : vec4s)    sp->SetUniform(name, v4);

        SubmitQuad(sp);
    }

    // ── DrawBorderRect ────────────────────────────────────────────────────────────

    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color)
    {
        s_flatColorShader->UseProgram();
        SetShaderProjection(s_flatColorShader);

        s_flatColorShader->SetUniform("u_Model", BuildQuadModel(pos, size, 0.0f, glm::vec2(0.0f)));
        s_flatColorShader->SetUniform("u_Color", color);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_LINESTRIP | BGFX_STATE_BLEND_ALPHA);

        bgfx::setVertexBuffer(0, s_quadVB);
        s_flatColorShader->Submit(ViewIdManager::GetCurrentId());
    }


    // ── MeasureText ───────────────────────────────────────────────────────────────
    // Returns the bounding box of the rendered text in atlas pixels.
    // Glyphs that are not yet in the atlas are added on demand (same as DrawText).
    // Makes NO bgfx calls (unlike DrawText) — it only touches the CPU-side
    // glyph cache — so it can safely be called from a different thread than
    // DrawText (e.g. a layout thread), as long as it takes atlas->mutex
    // exactly like DrawText does, which it does below.

    glm::vec2 MeasureText(const std::string& text, FontHandle fontHandle)
    {
        if (text.empty() || fontHandle == INVALID_FONT) return glm::vec2(0.f);

        FontAtlas* atlas = nullptr;
        {
            std::lock_guard<std::mutex> lock(s_fontMutex);
            auto it = s_fontRegistry.find(fontHandle);
            if (it == s_fontRegistry.end()) return glm::vec2(0.f);
            atlas = it->second;
        }

        // FIX: guard the glyph cache against a concurrent DrawText call on
        // this same atlas from the render thread.
        std::lock_guard<std::mutex> atlasLock(atlas->mutex);

        // Ensure every glyph is present so advance values are available
        {
            const char* p = text.c_str();
            while (*p) {
                const int cp = NextCodepoint(p);
                if (cp > 0 && cp != '\n')
                    atlas->EnsureGlyph(cp);
            }
        }

        const float lineH = atlas->LineHeight();

        float maxLineW = 0.f;
        float lineW = 0.f;
        int   numLines = 1;
        int   prevCp = 0;

        const char* p = text.c_str();
        while (*p) {
            const int cp = NextCodepoint(p);
            if (cp <= 0) continue;

            if (cp == '\n') {
                if (lineW > maxLineW) maxLineW = lineW;
                lineW = 0.f;
                prevCp = 0;
                ++numLines;
                continue;
            }

            const auto it = atlas->glyphs.find(cp);
            if (it == atlas->glyphs.end()) continue;

            if (prevCp != 0)
                lineW += stbtt_GetCodepointKernAdvance(&atlas->fontInfo, prevCp, cp) * atlas->scale;

            lineW += it->second.advanceX;
            prevCp = cp;
        }
        if (lineW > maxLineW) maxLineW = lineW;

        return glm::vec2(maxLineW, static_cast<float>(numLines) * lineH);
    }

    // ── EndFrame ──────────────────────────────────────────────────────────────────
    // Uploads any atlas changes that accumulated this frame. Must run on
    // the same thread as DrawText (it calls bgfx::updateTexture2D via
    // FlushToGPU).

    void EndFrame()
    {
        currentTime = Time::GameTimeNoPause;

        std::lock_guard<std::mutex> lock(s_fontMutex);
        for (auto& [id, atlas] : s_fontRegistry)
        {
            // FIX: FlushToGPU reads `pixels` and the dirty rect, both of
            // which EnsureGlyph (e.g. via a concurrent MeasureText call)
            // could be mutating right now for this atlas.
            std::lock_guard<std::mutex> atlasLock(atlas->mutex);
            atlas->FlushToGPU();
        }
    }

    // ── PushMask ──────────────────────────────────────────────────────────────────
// Draws `rect` into the stencil buffer at depth (stack size + 1).
// All subsequent draw calls will be clipped to this region until PopMask().
// Masks nest: each level intersects with all outer masks.

    void PushMask(const glm::vec2& pos, const glm::vec2& size,
        float rotation, glm::vec2 pivot)
    {
        const uint8_t   newDepth = static_cast<uint8_t>(s_maskStack.size() + 1);
        const glm::mat4 model = BuildQuadModel(pos, size, rotation, pivot);
        s_maskStack.push_back({ model });
        DrawStencilRect_Internal(model, newDepth);
    }

    void PushMask(const glm::mat3& transform, const glm::vec2& size)
    {
        const uint8_t   newDepth = static_cast<uint8_t>(s_maskStack.size() + 1);
        const glm::mat4 model = BuildQuadModelFromMat3(transform, size);
        s_maskStack.push_back({ model });
        DrawStencilRect_Internal(model, newDepth);
    }

    void PopMask()
    {
        if (s_maskStack.empty()) return;
        const MaskEntry& e = s_maskStack.back();
        const uint8_t    prevDepth = static_cast<uint8_t>(s_maskStack.size() - 1);
        DrawStencilRect_Internal(e.model, prevDepth);
        s_maskStack.pop_back();
    }

    void ClearStencil()
    {
        s_maskStack.clear();
        float screenH = static_cast<float>(UiManager::GetScaledUiHeight());
        float screenW = screenH * Camera::AspectRatio;
        if (customViewport) {
            screenW = static_cast<float>(customViewportSize.x);
            screenH = static_cast<float>(customViewportSize.y);
        }
        DrawStencilRect_Internal(
            BuildQuadModel({ 0.f, 0.f }, { screenW, screenH }, 0.f, { 0.f, 0.f }), 0);
    }

    // ── Matrix-based draw overloads ───────────────────────────────────────────

    void DrawTexturedRect(const glm::mat3& transform, const glm::vec2& size,
        bgfx::TextureHandle texture, const glm::vec4& color)
    {
        s_texturedShader->UseProgram();
        SetShaderProjection(s_texturedShader);
        s_texturedShader->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        s_texturedShader->SetUniform("u_Color", color);
        s_texturedShader->SetTexture("u_Texture", texture);
        s_texturedShader->SetUniform("bilboard", customViewport);
        SubmitQuad(s_texturedShader);
    }

    void DrawTexturedRectShader(const glm::mat3& transform, const glm::vec2& size,
        bgfx::TextureHandle texture, const glm::vec4& color, const string& shader, float textureHeight, float textureWidth)
    {
        auto* sp = ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);
        sp->SetUniform("u_ElementSize", glm::vec4(size.x, size.y, 0.f, 0.f));

        vec4 textureSize = {
            (float)textureWidth, (float)textureHeight,
            1.0f / textureWidth, 1.0f / textureHeight
        };

        sp->SetUniform("u_TextureSize", textureSize);
        SubmitQuad(sp);
    }

    void DrawTexturedRectShaderParams(const glm::mat3& transform, const glm::vec2& size,
        std::unordered_map<std::string, bgfx::TextureHandle>& textures,
        std::unordered_map<std::string, vec4>& vec4s,
        const glm::vec4& color, const string& shader)
    {
        auto* sp = ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetUniform("u_ElementSize", glm::vec4(size.x, size.y, 0.f, 0.f));
        for (auto& [name, tex] : textures) sp->SetTexture(name, tex);
        for (auto& [name, v4] : vec4s)    sp->SetUniform(name, v4);
        SubmitQuad(sp);
    }


    void DrawBorderRect(const glm::mat3& transform, const glm::vec2& size,
        const glm::vec4& color)
    {
        s_flatColorShader->UseProgram();
        SetShaderProjection(s_flatColorShader);
        s_flatColorShader->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        s_flatColorShader->SetUniform("u_Color", color);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_LINESTRIP | BGFX_STATE_BLEND_ALPHA);
        bgfx::setVertexBuffer(0, s_quadVB);
        s_flatColorShader->Submit(ViewIdManager::GetCurrentId());
    }

    void DrawText(std::string text, FontHandle fontHandle,
        const glm::mat3& transform,
        const glm::vec4& color, const glm::vec2& scale,
        const string& shader, std::unordered_map<std::string, vec4> shaderUniforms, float effectPadding)   // extra bleed, in VIEWPORT PIXELS, needed by outline/glow/shadow
    {
        if (text.empty() || fontHandle == INVALID_FONT) return;

        FontAtlas* atlas = nullptr;
        {
            std::lock_guard<std::mutex> lock(s_fontMutex);
            auto it = s_fontRegistry.find(fontHandle);
            if (it == s_fontRegistry.end()) return;
            atlas = it->second;
        }

        // FIX: guards the glyph cache (packX/packY/rowH/pixels/glyphs/dirty
        // rect) against a concurrent MeasureText call on this same atlas
        // from another thread.
        std::lock_guard<std::mutex> atlasLock(atlas->mutex);

        // FIX: this is the actual fix for "fonts loading in parallel to a
        // font being drawn." LoadFont() no longer creates the bgfx texture
        // — it's created here instead, lazily, on first draw. Since
        // DrawText is confirmed single-threaded, this guarantees
        // bgfx::createTexture2D for every font atlas runs on that same one
        // thread, no matter which thread actually called LoadFont(). This
        // is idempotent (CreateGpuResources no-ops once the texture
        // exists), so the cost after the first draw is one bgfx::isValid()
        // check.
        if (!bgfx::isValid(atlas->texture)) {
            if (!atlas->CreateGpuResources()) {
                std::cerr << "[UiRenderer] DrawText: failed to create GPU texture for font atlas\n";
                return;
            }
        }

        // effectPadding arrives in VIEWPORT PIXELS (see UiElement::GetEffectsPadding),
        // consistent with every other draw call in this file. Convert to the atlas's
        // own native pixel space (pre-`scale`) since that's the space the geometry/UV
        // math below (penX/penY/g.bitmapW/etc) already operates in — without this, a
        // DrawText call using scale != (1,1) would inflate by the wrong amount.
        const float atlasPadX = (scale.x > 0.f) ? effectPadding / scale.x : effectPadding;
        const float atlasPadY = (scale.y > 0.f) ? effectPadding / scale.y : effectPadding;

        // atlas pixel size — needed to convert padding into UV space below.
        const float atlasW = static_cast<float>(FontAtlas::ATLAS_W);
        const float atlasH = static_cast<float>(FontAtlas::ATLAS_H);

        // The atlas only physically reserves atlas->padding texels of blank space
        // between packed glyphs (see FontAtlas::padding) — just enough to keep
        // bilinear filtering from blending across a glyph boundary, not sized for an
        // arbitrarily large effect radius. Geometry and UV below extend by the FULL
        // atlasPadX/Y — so a big shadow/glow gets its correct on-screen reach, and
        // (just as importantly) the glyph's own tight ink isn't distorted, since
        // geometry and UV need to share one consistent position→UV scale across the
        // whole quad (see the matching comment on DrawTexturedRect9Slice's padding for
        // why mixing scales there would be wrong). The per-glyph u_ClampRect that
        // actually gates sampling, below, stays capped at this smaller safe margin
        // instead: past that cap, fs_effects.sc's sampleTexClamped() returns
        // transparent, so an oversized effect just fades out sooner than requested
        // instead of bleeding into whatever glyph happens to be packed next door.
        const float clampPadX = std::min(atlasPadX, static_cast<float>(atlas->padding));
        const float clampPadY = std::min(atlasPadY, static_cast<float>(atlas->padding));

        // Pass 1: ensure all glyphs
        {
            const char* p = text.c_str();
            while (*p) {
                const int cp = NextCodepoint(p);
                if (cp > 0 && cp != '\n') atlas->EnsureGlyph(cp);
            }
        }

        // Pass 2: measure (unaffected by padding — layout must stay identical)
        const float lineH = atlas->LineHeight();
        const float baseline = atlas->BaselineOff();
        float maxLineW = 0.f, lineW = 0.f;
        int   numLines = 1, numGlyphs = 0;
        {
            const char* p = text.c_str();
            int prevCp = 0;
            while (*p) {
                const int cp = NextCodepoint(p);
                if (cp <= 0) continue;
                if (cp == '\n') {
                    if (lineW > maxLineW) maxLineW = lineW;
                    lineW = 0.f; prevCp = 0; ++numLines; continue;
                }
                const auto it = atlas->glyphs.find(cp);
                if (it == atlas->glyphs.end()) continue;
                const GlyphInfo& g = it->second;
                if (prevCp != 0)
                    lineW += stbtt_GetCodepointKernAdvance(&atlas->fontInfo, prevCp, cp) * atlas->scale;
                lineW += g.advanceX; prevCp = cp;
                if (!g.invisible) ++numGlyphs;
            }
            if (lineW > maxLineW) maxLineW = lineW;
        }
        if (maxLineW <= 0.f || numGlyphs == 0) return;

        const float textW = maxLineW;
        const float textH = static_cast<float>(numLines) * lineH;

        // Pass 3: build TVB
        const uint32_t vertexCount = static_cast<uint32_t>(numGlyphs * 6);
        if (bgfx::getAvailTransientVertexBuffer(vertexCount, s_quadLayout) < vertexCount) {
            std::cerr << "[UiRenderer] DrawText(mat3): not enough transient VB\n";
            return;
        }
        bgfx::TransientVertexBuffer tvb;
        bgfx::allocTransientVertexBuffer(&tvb, vertexCount, s_quadLayout);
        auto* v = reinterpret_cast<QuadVertex*>(tvb.data);

        const float padU = atlasPadX / atlasW;
        const float padV = atlasPadY / atlasH;
        const float clampPadU = clampPadX / atlasW;
        const float clampPadV = clampPadY / atlasH;

        // One entry per emitted (visible) glyph, same order they're written
        // to the VB — Pass 4 uses this as each glyph's own u_ClampRect when
        // effects are active, so one glyph's shadow/glow/outline can't
        // sample into whatever's packed next to it in the shared atlas.
        std::vector<glm::vec4> glyphClampRects;
        glyphClampRects.reserve(numGlyphs);

        float penX = 0.f, penY = 0.f;
        int   prevCp = 0;
        const char* p = text.c_str();
        while (*p) {
            const int cp = NextCodepoint(p);
            if (cp <= 0) continue;
            if (cp == '\n') { penX = 0.f; penY += lineH; prevCp = 0; continue; }
            const auto it = atlas->glyphs.find(cp);
            if (it == atlas->glyphs.end()) continue;
            const GlyphInfo& g = it->second;
            if (prevCp != 0)
                penX += stbtt_GetCodepointKernAdvance(&atlas->fontInfo, prevCp, cp) * atlas->scale;
            if (!g.invisible) {
                // geometry: inflate the quad around the glyph's ink, pen position untouched
                const float lx = penX + static_cast<float>(g.xoff) - atlasPadX;
                const float ly = penY + baseline + static_cast<float>(g.yoff) - atlasPadY;
                const float rw = static_cast<float>(g.bitmapW) + atlasPadX * 2.f;
                const float rh = static_cast<float>(g.bitmapH) + atlasPadY * 2.f;
                const float nx = lx / textW, ny = ly / textH;
                const float nrw = rw / textW, nrh = rh / textH;

                // UV: inflate by the SAME atlasPadX/Y as geometry above (not the
                // smaller, clamp-capped amount) — using one consistent scale for the
                // whole quad keeps the glyph's own tight ink undistorted, not just its
                // padding border. sampleTexClamped()'s clamp-rect check (below) is what
                // actually limits how far a sample is allowed to reach, independent of
                // how far this UV range itself extends.
                const float u0 = g.u0 - padU, u1 = g.u1 + padU;
                const float v0 = g.v0 - padV, v1 = g.v1 + padV;

                v[0] = { nx,       ny + nrh, u0, v1 };
                v[1] = { nx + nrw, ny,       u1, v0 };
                v[2] = { nx,       ny,       u0, v0 };
                v[3] = { nx,       ny + nrh, u0, v1 };
                v[4] = { nx + nrw, ny + nrh, u1, v1 };
                v[5] = { nx + nrw, ny,       u1, v0 };
                v += 6;

                // Clamp rect stays capped at the atlas's real, physically-safe margin
                // (clampPadU/V) regardless of how far the geometry/UV above actually
                // extends — see the atlasPadX/clampPadX comment further up.
                glyphClampRects.emplace_back(g.u0 - clampPadU, g.v0 - clampPadV,
                                              g.u1 + clampPadU, g.v1 + clampPadV);
            }
            penX += g.advanceX; prevCp = cp;
        }

        // Pass 4: submit
        const glm::vec2 drawSize(scale.x * textW, scale.y * textH);
        Shader* sp = shader.empty()
            ? s_texturedShader
            : ShaderManager::GetShaderProgram("vs_ui", shader);
        const glm::mat4 model = BuildQuadModelFromMat3(transform, drawSize);

        if (atlas->textureDirty) {
            atlas->FlushToGPU();
        }

        if (shader.empty())
        {
            // No effects: exactly the original single-draw-call path, whole
            // string in one submit. u_ClampRect doesn't exist in the plain
            // textured shader, so there's nothing to set per glyph here.
            sp->UseProgram();
            SetShaderProjection(sp);
            sp->SetUniform("u_Model", model);
            sp->SetUniform("u_Color", color);
            sp->SetTexture("u_Texture", atlas->texture);

            bgfx::setState(
                BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                BGFX_STATE_DEPTH_TEST_ALWAYS |
                BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                    BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
            ApplyStencilTest();
            bgfx::setVertexBuffer(0, &tvb);
            sp->Submit(ViewIdManager::GetCurrentId());
            return;
        }

        // Effects active: one draw call per glyph, each with its own
        // u_ClampRect (see fs_effects.sc) so a glow/shadow/outline radius
        // large enough to reach past a tightly-packed glyph's own cell
        // still can't pick up a neighboring glyph's pixels. Costs
        // numGlyphs draw calls instead of 1 — text with effects enabled is
        // usually short (labels, buttons, headers), so this is a reasonable
        // trade for correctness, but it's worth knowing about for very long
        // shadowed/glowing strings.
        for (int i = 0; i < numGlyphs; ++i)
        {
            sp->UseProgram();
            SetShaderProjection(sp);
            sp->SetUniform("u_Model", model);
            sp->SetUniform("u_Color", color);
            sp->SetTexture("u_Texture", atlas->texture);
            sp->SetUniform("u_ElementSize", glm::vec4(drawSize.x, drawSize.y, 0.f, 0.f));

            for (const auto& pair : shaderUniforms)
                sp->SetUniform(pair.first, pair.second);

            // Overrides the u_ClampRect that came in via shaderUniforms
            // (GetEffectsUniforms()'s default is (0,0,1,1), the whole
            // atlas — wrong for text specifically) with this glyph's own.
            sp->SetUniform("u_ClampRect", glyphClampRects[i]);

            bgfx::setState(
                BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                BGFX_STATE_DEPTH_TEST_ALWAYS |
                BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                    BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
            ApplyStencilTest();
            bgfx::setVertexBuffer(0, &tvb, i * 6, 6);
            sp->Submit(ViewIdManager::GetCurrentId());
        }
    }

    // ── DrawTexturedRectRegion ────────────────────────────────────────────────
    // See UiRenderer.h for the full explanation. Geometry and UV both span
    // [rectPos, rectPos+rectSize], optionally extended by effectPadding — the
    // same trick DrawText's Pass 3 uses per-glyph, generalized to one rect.
    void DrawTexturedRectRegion(const glm::mat3& transform, const glm::vec2& size,
        const glm::vec2& rectPos, const glm::vec2& rectSize,
        bgfx::TextureHandle texture, const glm::vec4& color,
        const string& shader, std::unordered_map<std::string, vec4> shaderUniforms,
        float effectPadding, float textureWidth, float textureHeight)
    {
        if (rectSize.x <= 0.f || rectSize.y <= 0.f) return;

        // effectPadding arrives in VIEWPORT PIXELS (see UiElement::GetEffectsPadding)
        // — convert to this element's local [0,1] space by dividing by its own
        // on-screen size, so the padding geometry/UV extend by the right amount
        // regardless of the bound texture's actual resolution (fs_effects.sc's own
        // ring-sampling radius is handled separately, via screen-space derivatives —
        // see pixelsToUV() in fs_effects.sc).
        const float padU = (size.x > 0.f) ? effectPadding / size.x : 0.f;
        const float padV = (size.y > 0.f) ? effectPadding / size.y : 0.f;

        const float x0 = rectPos.x - padU, x1 = rectPos.x + rectSize.x + padU;
        const float y0 = rectPos.y - padV, y1 = rectPos.y + rectSize.y + padV;

        Shader* sp = shader.empty() ? s_texturedShader : ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);
        sp->SetUniform("u_ElementSize", glm::vec4(size.x, size.y, 0.f, 0.f));
        (void)textureWidth;
        (void)textureHeight;

        for (const auto& pair : shaderUniforms)
            sp->SetUniform(pair.first, pair.second);

        if (bgfx::getAvailTransientVertexBuffer(6, s_quadLayout) < 6) {
            std::cerr << "[UiRenderer] DrawTexturedRectRegion: not enough transient VB\n";
            return;
        }
        bgfx::TransientVertexBuffer tvb;
        bgfx::allocTransientVertexBuffer(&tvb, 6, s_quadLayout);
        auto* v = reinterpret_cast<QuadVertex*>(tvb.data);

        // u_Model already maps this element's local [0,1] space to its full
        // screen-space rect, so submitting geometry for a sub-range (here,
        // optionally extended past [0,1] for padding) draws exactly that
        // fraction, sampling the matching fraction (or bleed) of the texture.
        v[0] = { x0, y1, x0, y1 };
        v[1] = { x1, y0, x1, y0 };
        v[2] = { x0, y0, x0, y0 };
        v[3] = { x0, y1, x0, y1 };
        v[4] = { x1, y1, x1, y1 };
        v[5] = { x1, y0, x1, y0 };

        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
        BgfxStateManager::Apply();

        ApplyStencilTest();
        bgfx::setVertexBuffer(0, &tvb);
        sp->Submit(ViewIdManager::GetCurrentId());
    }

    // ── DrawTexturedRect9Slice ────────────────────────────────────────────────
    void DrawTexturedRect9Slice(const glm::mat3& transform, const glm::vec2& size,
        const NineSliceMargins& margins,
        bgfx::TextureHandle texture, const glm::vec4& color,
        const string& shader, std::unordered_map<std::string, vec4> shaderUniforms,
        float effectPadding, float textureWidth, float textureHeight)
    {
        if (size.x <= 0.f || size.y <= 0.f) return;

        // effectPadding (viewport pixels) extends only the OUTER boundary of the
        // whole 9-slice grid — the inner grid lines (gx1/gx2/gy1/gy2 and
        // ux1/ux2/uy1/uy2 below) never move, so corner/edge/center proportions
        // the element was actually authored with stay exactly as configured; the
        // whole grid just gets `effectPadding` extra screen pixels of margin on
        // all 4 sides for shadow/outline/glow to bleed into.
        //
        // This is safe to convert 1:1 (viewport pixels == texture texels) at the
        // outer boundary specifically, because every cell touching it is either a
        // corner (native 1:1 scale in both axes by 9-slice construction) or an
        // edge cell in the direction perpendicular to its own stretch (which is
        // exactly the direction of the outer boundary it touches, e.g. the top
        // edge cell is stretched horizontally but still native vertically) — so
        // there's no separate "convert to element-local space" step needed the
        // way DrawTexturedRectRegion needs one; the padding is already correct in
        // real texture texels.
        const float padGX = (size.x > 0.f) ? effectPadding / size.x : 0.f;
        const float padGY = (size.y > 0.f) ? effectPadding / size.y : 0.f;
        const float padUX = (textureWidth  > 0.f) ? effectPadding / textureWidth  : 0.f;
        const float padUY = (textureHeight > 0.f) ? effectPadding / textureHeight : 0.f;

        // Geometry breakpoints, local [0,1] space relative to `size` — corners
        // come out `margin` SCREEN pixels regardless of how `size` is
        // stretched (that's the entire point of 9-slicing). Clamped so two
        // opposite margins can't cross on a too-small element.
        float gx1 = (size.x > 0.f) ? margins.left / size.x : 0.f;
        float gx2 = (size.x > 0.f) ? 1.f - margins.right / size.x : 1.f;
        float gy1 = (size.y > 0.f) ? margins.top / size.y : 0.f;
        float gy2 = (size.y > 0.f) ? 1.f - margins.bottom / size.y : 1.f;
        if (gx1 > gx2) { const float m = (gx1 + gx2) * 0.5f; gx1 = gx2 = m; }
        if (gy1 > gy2) { const float m = (gy1 + gy2) * 0.5f; gy1 = gy2 = m; }

        // UV breakpoints, texture-space fraction — independent of `size`,
        // purely margins-in-texels over textureWidth/Height.
        const float ux1 = (textureWidth > 0.f) ? margins.left / textureWidth : 0.f;
        const float ux2 = (textureWidth > 0.f) ? 1.f - margins.right / textureWidth : 1.f;
        const float uy1 = (textureHeight > 0.f) ? margins.top / textureHeight : 0.f;
        const float uy2 = (textureHeight > 0.f) ? 1.f - margins.bottom / textureHeight : 1.f;

        // Only the OUTER breakpoints (index 0 and 3) move for padding — see the
        // comment on padGX/padGY/padUX/padUY above. sampleTexClamped() in
        // fs_effects.sc keeps the padded UV from ever reading real content past
        // the source texture's own edge (u_ClampRect defaults to (0,0,1,1) for a
        // 9-sliced draw, same as a plain image — see UiElement::GetEffectsUniforms).
        const float gx[4] = { -padGX, gx1, gx2, 1.f + padGX };
        const float gy[4] = { -padGY, gy1, gy2, 1.f + padGY };
        const float ux[4] = { -padUX, ux1, ux2, 1.f + padUX };
        const float uy[4] = { -padUY, uy1, uy2, 1.f + padUY };

        Shader* sp = shader.empty() ? s_texturedShader : ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);
        sp->SetUniform("u_ElementSize", glm::vec4(size.x, size.y, 0.f, 0.f));

        for (const auto& pair : shaderUniforms)
            sp->SetUniform(pair.first, pair.second);

        constexpr uint32_t vertexCount = 9 * 6;
        if (bgfx::getAvailTransientVertexBuffer(vertexCount, s_quadLayout) < vertexCount) {
            std::cerr << "[UiRenderer] DrawTexturedRect9Slice: not enough transient VB\n";
            return;
        }
        bgfx::TransientVertexBuffer tvb;
        bgfx::allocTransientVertexBuffer(&tvb, vertexCount, s_quadLayout);
        auto* v = reinterpret_cast<QuadVertex*>(tvb.data);

        for (int cy = 0; cy < 3; ++cy)
        {
            for (int cx = 0; cx < 3; ++cx)
            {
                const float x0 = gx[cx], x1 = gx[cx + 1];
                const float y0 = gy[cy], y1 = gy[cy + 1];
                const float u0 = ux[cx], u1 = ux[cx + 1];
                const float t0 = uy[cy], t1 = uy[cy + 1];

                v[0] = { x0, y1, u0, t1 };
                v[1] = { x1, y0, u1, t0 };
                v[2] = { x0, y0, u0, t0 };
                v[3] = { x0, y1, u0, t1 };
                v[4] = { x1, y1, u1, t1 };
                v[5] = { x1, y0, u1, t0 };
                v += 6;
            }
        }

        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
        BgfxStateManager::Apply();

        ApplyStencilTest();
        bgfx::setVertexBuffer(0, &tvb);
        sp->Submit(ViewIdManager::GetCurrentId());
    }

} // namespace UiRenderer