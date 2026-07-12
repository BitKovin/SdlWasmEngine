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
    static constexpr int ATLAS_W = 2048;
    static constexpr int ATLAS_H = 2048;
    std::vector<uint8_t> pixels;         // ATLAS_W * ATLAS_H * 4 (RGBA8)
    bgfx::TextureHandle  texture = BGFX_INVALID_HANDLE;
    bool textureDirty = false;

    // ── Packing & Padding configuration ──────────────────────────────────────
    int padding = 10;
    int packX = 2; // Initialized dynamically in Init() based on padding
    int packY = 2;
    int rowH = 0; // tallest glyph in the current row

    // ── Glyph cache ───────────────────────────────────────────────────────────
    std::unordered_map<int, GlyphInfo> glyphs;

    // ── Init / Destroy ────────────────────────────────────────────────────────

    
    bool Init(const char* path, float height, int paddingSize = 5)
    {
        padding = height / 2 + paddingSize;
        packX = padding;
        packY = padding;
        rowH = 0;

        // Read the entire .ttf file into memory
        FILE* f = std::fopen(path, "rb");
        if (!f) {
            std::cerr << "[UiRenderer] LoadFont: cannot open '" << path << "'\n";
            return false;
        }
        std::fseek(f, 0, SEEK_END);
        const long sz = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        fileData.resize(static_cast<size_t>(sz));
        std::fread(fileData.data(), 1, static_cast<size_t>(sz), f);
        std::fclose(f);

        if (!stbtt_InitFont(&fontInfo, fileData.data(), 0)) {
            std::cerr << "[UiRenderer] LoadFont: stbtt_InitFont failed for '" << path << "'\n";
            fileData.clear();
            return false;
        }

        pixelHeight = height;
        scale = stbtt_ScaleForPixelHeight(&fontInfo, height);
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

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

        // No mipmaps. bgfx default sampler = bilinear filtering + clamp.
        texture = bgfx::createTexture2D(
            static_cast<uint16_t>(ATLAS_W),
            static_cast<uint16_t>(ATLAS_H),
            false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
            nullptr);

        textureDirty = false;
        return true;
    }

    void Destroy()
    {
        if (bgfx::isValid(texture)) {
            bgfx::destroy(texture);
            texture = BGFX_INVALID_HANDLE;
        }
        fileData.clear();
        pixels.clear();
        glyphs.clear();
    }

    // ── EnsureGlyph ───────────────────────────────────────────────────────────

    bool EnsureGlyph(int codepoint)
    {
        if (glyphs.count(codepoint))
            return true;

        // Rasterize the glyph into a temporary 1-channel bitmap
        int w = 0, h = 0, xoff = 0, yoff = 0;
        uint8_t* bm = stbtt_GetCodepointBitmap(
            &fontInfo, 0.f, scale, codepoint, &w, &h, &xoff, &yoff);

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
            GlyphInfo g{};
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

    void FlushToGPU()
    {
        if (!textureDirty || !bgfx::isValid(texture))
            return;

        const uint32_t byteCount = static_cast<uint32_t>(ATLAS_W) * ATLAS_H * 4;
        bgfx::updateTexture2D(texture, 0, 0, 0, 0,
            static_cast<uint16_t>(ATLAS_W),
            static_cast<uint16_t>(ATLAS_H),
            bgfx::copy(pixels.data(), byteCount));
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
        if (!atlas->Init(path, pixelHeight)) {
            delete atlas;
            return INVALID_FONT;
        }

        const FontHandle id = s_nextFontId++;
        s_fontRegistry[id] = atlas;
        s_fontKeyCache[key] = id;
        return id;
    }

    // ── UnloadFont ────────────────────────────────────────────────────────────────

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
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
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
    // Uploads any atlas changes that accumulated this frame.

    void EndFrame()
    {
        currentTime = Time::GameTimeNoPause;

        std::lock_guard<std::mutex> lock(s_fontMutex);
        for (auto& [id, atlas] : s_fontRegistry)
            atlas->FlushToGPU();
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
                  const string& shader, std::unordered_map<std::string, vec4> shaderUniforms, float effectPadding)   // extra bleed, in atlas texels, needed by outline/glow/shadow
    {
        if (text.empty() || fontHandle == INVALID_FONT) return;

        FontAtlas* atlas = nullptr;
        {
            std::lock_guard<std::mutex> lock(s_fontMutex);
            auto it = s_fontRegistry.find(fontHandle);
            if (it == s_fontRegistry.end()) return;
            atlas = it->second;
        }

        // atlas pixel size — needed to convert padding into UV space, and to
        // feed u_TextureSize to the effects shader. Adjust field names if your
        // FontAtlas stores them differently.
        const float atlasW = static_cast<float>(FontAtlas::ATLAS_W);
        const float atlasH = static_cast<float>(FontAtlas::ATLAS_H);

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

        const float padU = effectPadding / atlasW;
        const float padV = effectPadding / atlasH;

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
                const float lx = penX + static_cast<float>(g.xoff) - effectPadding;
                const float ly = penY + baseline + static_cast<float>(g.yoff) - effectPadding;
                const float rw = static_cast<float>(g.bitmapW) + effectPadding * 2.f;
                const float rh = static_cast<float>(g.bitmapH) + effectPadding * 2.f;
                const float nx = lx / textW, ny = ly / textH;
                const float nrw = rw / textW, nrh = rh / textH;

                // UV: inflate to match, so the shader has real atlas pixels to sample for the halo
                const float u0 = g.u0 - padU, u1 = g.u1 + padU;
                const float v0 = g.v0 - padV, v1 = g.v1 + padV;

                v[0] = { nx,       ny + nrh, u0, v1 };
                v[1] = { nx + nrw, ny,       u1, v0 };
                v[2] = { nx,       ny,       u0, v0 };
                v[3] = { nx,       ny + nrh, u0, v1 };
                v[4] = { nx + nrw, ny + nrh, u1, v1 };
                v[5] = { nx + nrw, ny,       u1, v0 };
                v += 6;

                glyphClampRects.emplace_back(u0, v0, u1, v1);
            }
            penX += g.advanceX; prevCp = cp;
        }

        // Pass 4: submit
        const glm::vec2 drawSize(scale.x * textW, scale.y * textH);
        Shader* sp = shader.empty()
            ? s_texturedShader
            : ShaderManager::GetShaderProgram("vs_ui", shader);
        const glm::mat4 model = BuildQuadModelFromMat3(transform, drawSize);

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
            sp->SetUniform("u_TextureSize", glm::vec4(atlasW, atlasH, 0.f, 0.f));

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

        const float padU = (textureWidth  > 0.f) ? effectPadding / textureWidth  : 0.f;
        const float padV = (textureHeight > 0.f) ? effectPadding / textureHeight : 0.f;

        const float x0 = rectPos.x - padU, x1 = rectPos.x + rectSize.x + padU;
        const float y0 = rectPos.y - padV, y1 = rectPos.y + rectSize.y + padV;

        Shader* sp = shader.empty() ? s_texturedShader : ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);

        if (!shader.empty() && textureWidth > 0.f && textureHeight > 0.f)
            sp->SetUniform("u_TextureSize", glm::vec4(textureWidth, textureHeight, 0.f, 0.f));

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

        // Unlike DrawTexturedRectRegion/DrawText, 9-slice geometry NEVER
        // extends past [0,size] — corners/edges/center always add up to
        // exactly `size`, full stop, whether or not effectPadding is > 0.
        // That means shadow/outline/glow on a 9-sliced element render
        // clipped to its own box rather than bleeding past it, which is a
        // real limitation compared to a plain UiImage — but a 9-slice's
        // whole point is a stable, predictable footprint (borders, panels,
        // buttons sized to fit content), and letting effects silently grow
        // that footprint undermines exactly what 9-slicing is for.
        //
        // No UV padding either, and deliberately not just "no geometry
        // padding" — padding only the UV while geometry stays fixed would
        // cram extra texture range into the same corner size, shrinking
        // the apparent content instead of leaving it at native scale. The
        // shader's own internal ring-sampling still reads slightly outside
        // each cell during blur/glow (clamped by u_ClampRect, see
        // fs_effects.sc); it just doesn't get a *dedicated* margin for it.

        // Geometry breakpoints, local [0,1] space relative to `size` — corners
        // come out `margin` SCREEN pixels regardless of how `size` is
        // stretched (that's the entire point of 9-slicing). Clamped so two
        // opposite margins can't cross on a too-small element.
        float gx1 = (size.x > 0.f) ? margins.left   / size.x : 0.f;
        float gx2 = (size.x > 0.f) ? 1.f - margins.right  / size.x : 1.f;
        float gy1 = (size.y > 0.f) ? margins.top    / size.y : 0.f;
        float gy2 = (size.y > 0.f) ? 1.f - margins.bottom / size.y : 1.f;
        if (gx1 > gx2) { const float m = (gx1 + gx2) * 0.5f; gx1 = gx2 = m; }
        if (gy1 > gy2) { const float m = (gy1 + gy2) * 0.5f; gy1 = gy2 = m; }

        // UV breakpoints, texture-space fraction — independent of `size`,
        // purely margins-in-texels over textureWidth/Height.
        const float ux1 = (textureWidth  > 0.f) ? margins.left   / textureWidth  : 0.f;
        const float ux2 = (textureWidth  > 0.f) ? 1.f - margins.right  / textureWidth  : 1.f;
        const float uy1 = (textureHeight > 0.f) ? margins.top    / textureHeight : 0.f;
        const float uy2 = (textureHeight > 0.f) ? 1.f - margins.bottom / textureHeight : 1.f;

        // Both outer bounds are exactly 0 / 1 — see comment above.
        const float gx[4] = { 0.f, gx1, gx2, 1.f };
        const float gy[4] = { 0.f, gy1, gy2, 1.f };
        const float ux[4] = { 0.f, ux1, ux2, 1.f };
        const float uy[4] = { 0.f, uy1, uy2, 1.f };

        // effectPadding is intentionally unused — kept in the signature to
        // match DrawTexturedRectRegion/DrawText's shape, but see the big
        // comment above for why 9-slice doesn't apply it.
        (void)effectPadding;

        Shader* sp = shader.empty() ? s_texturedShader : ShaderManager::GetShaderProgram("vs_ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);
        sp->SetUniform("u_Model", BuildQuadModelFromMat3(transform, size));
        sp->SetUniform("u_Color", color);
        sp->SetTexture("u_Texture", texture);

        if (!shader.empty() && textureWidth > 0.f && textureHeight > 0.f)
            sp->SetUniform("u_TextureSize", glm::vec4(textureWidth, textureHeight, 0.f, 0.f));

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