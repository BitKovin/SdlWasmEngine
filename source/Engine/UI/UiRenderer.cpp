#include "UiRenderer.h"
#include <bgfx/bgfx.h>
#include "../ShaderManager.h"
#include "../Camera.h"
#include <unordered_map>
#include <SDL2/SDL.h>
#include <iostream>
#include "../Time.hpp"
#include <mutex>
#include "UiManager.h"

#include <BgfxStateManager.h>

#include <Renderer/Abstractions/ViewIdManager.h>

// ─────────────────────────────────────────────────────────────────────────────
// Cache entry
// ─────────────────────────────────────────────────────────────────────────────

struct TextureCacheEntry {
    bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE; // bgfx texture handle
    float  lastUsedTime = 0.0f; // Last time used (seconds)
    size_t memorySize = 0;    // Memory size in bytes
    int    width = 0;    // Texture width for rendering
    int    height = 0;    // Texture height for rendering
};

// ─────────────────────────────────────────────────────────────────────────────
// Quad vertex layout & static GPU resources
// ─────────────────────────────────────────────────────────────────────────────

struct QuadVertex {
    float x, y; // position
    float u, v; // texcoord
};

static bgfx::VertexLayout       s_quadLayout;
static bgfx::VertexBufferHandle s_quadVB = BGFX_INVALID_HANDLE;
static Shader* s_texturedShader = nullptr;
static Shader* s_flatColorShader = nullptr;

// ─────────────────────────────────────────────────────────────────────────────
// Text-texture cache
// ─────────────────────────────────────────────────────────────────────────────

static std::unordered_map<std::string, TextureCacheEntry> textTextureCache;
static size_t       totalCacheMemory = 0;
static const size_t MAX_CACHE_MEMORY = 50 * 1024 * 1024; // 50 MB
static const float  MAX_UNUSED_SECONDS = 2.0f;
static float        currentTime = 0.0f;
static std::mutex   textTextureCacheMutex;

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

        // Unit quad [0,1]x[0,1], two CCW triangles, y-down origin
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
        if (bgfx::isValid(s_quadVB))
        {
            bgfx::destroy(s_quadVB);
            s_quadVB = BGFX_INVALID_HANDLE;
        }

        // Shaders are owned by ShaderManager – do not delete here.
        s_texturedShader = nullptr;
        s_flatColorShader = nullptr;

        // Destroy all cached text textures
        {
            std::lock_guard<std::mutex> lock(textTextureCacheMutex);
            for (auto& pair : textTextureCache)
            {
                if (bgfx::isValid(pair.second.textureHandle))
                    bgfx::destroy(pair.second.textureHandle);
            }
            textTextureCache.clear();
            totalCacheMemory = 0;
        }
    }

    // ── Projection helper ─────────────────────────────────────────────────────────

    static void SetShaderProjection(Shader* shader)
    {
        float screenHeight = static_cast<float>(UiManager::GetScaledUiHeight());
        float screenWidth = screenHeight * Camera::AspectRatio;

        if (customViewport)
        {
            screenWidth = static_cast<float>(customViewportSize.x);
            screenHeight = static_cast<float>(customViewportSize.y);
        }

        // Orthographic projection: top-left origin, y-down
        glm::mat4 uiProjection = glm::ortho(
            0.0f, screenWidth,
            screenHeight, 0.0f,
            -1.0f, 1.0f);

        shader->SetUniform("u_Projection", uiProjection);
    }

    // ── Shared model-matrix builder ───────────────────────────────────────────────

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

    // ── Shared submit: bind the unit quad VB and dispatch ────────────────────────

    static void SubmitQuad(Shader* shader)
    {
        
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
        BgfxStateManager::Apply();

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

        SubmitQuad(sp);
    }

    // ── DrawTexturedRectShaderParams ──────────────────────────────────────────────

    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size,
        float rotation, glm::vec2 pivot,
        std::unordered_map<std::string, bgfx::TextureHandle>& textures,
        std::unordered_map<std::string, float>& scalars,
        std::unordered_map<std::string, vec4>& vec4s,
        const glm::vec4& color, const string& shader)
    {
        auto* sp = ShaderManager::GetShaderProgram("ui", shader);
        sp->UseProgram();
        SetShaderProjection(sp);

        sp->SetUniform("u_Model", BuildQuadModel(pos, size, rotation, pivot));
        sp->SetUniform("u_Color", color);

        for (auto& tex : textures) sp->SetTexture(tex.first, tex.second);
        for (auto& scalar : scalars)  sp->SetUniform(scalar.first, scalar.second);
        for (auto& v4 : vec4s)    sp->SetUniform(v4.first, v4.second);

        SubmitQuad(sp);
    }

    // ── DrawBorderRect ────────────────────────────────────────────────────────────
    // bgfx has no glPolygonMode equivalent; wireframe is expressed as a render-
    // state override (BGFX_STATE_PT_LINESTRIP) passed before Submit.

    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color)
    {
        s_flatColorShader->UseProgram();
        SetShaderProjection(s_flatColorShader);

        s_flatColorShader->SetUniform("u_Model", BuildQuadModel(pos, size, 0.0f, glm::vec2(0.0f)));
        s_flatColorShader->SetUniform("u_Color", color);

        // Wireframe: write colour only, alpha-blend, line topology
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_PT_LINESTRIP | BGFX_STATE_BLEND_ALPHA);

        bgfx::setVertexBuffer(0, s_quadVB);
        s_flatColorShader->Submit(ViewIdManager::GetCurrentId());
    }

    // ── DrawText ──────────────────────────────────────────────────────────────────

    void DrawText(std::string text, TTF_Font* font,
        const glm::vec2& pos, float rotation, glm::vec2 pivot,
        const glm::vec4& color, const glm::vec2& scale,
        const std::string& shader)
    {
        if (!font)
        {
            std::cerr << "No font provided for DrawText." << std::endl;
            return;
        }
        if (text.empty()) return;

        bgfx::TextureHandle cachedHandle = BGFX_INVALID_HANDLE;
        int textureWidth = 0;
        int textureHeight = 0;

        // ── Cache lookup ──────────────────────────────────────────────────────────
        {
            std::lock_guard<std::mutex> lock(textTextureCacheMutex);
            auto it = textTextureCache.find(text);
            if (it != textTextureCache.end())
            {
                cachedHandle = it->second.textureHandle;
                textureWidth = it->second.width;
                textureHeight = it->second.height;
                it->second.lastUsedTime = currentTime;
            }
        }

        // ── Cache miss: rasterise via SDL_TTF and upload to bgfx ─────────────────
        if (!bgfx::isValid(cachedHandle))
        {
            // Render the glyph at the requested tint colour; the shader will
            // multiply it again by u_Color – use white here if you prefer.
            SDL_Color sdlColor = {
                static_cast<Uint8>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f)
            };

            SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), sdlColor, 0);
            if (!surface)
            {
                std::cerr << "TTF_RenderUTF8_Blended Error: " << TTF_GetError() << std::endl;
                return;
            }

            // Normalise to contiguous RGBA32 before uploading
            SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
            if (!formatted)
                formatted = surface; // fallback

            const int    w = formatted->w;
            const int    h = formatted->h;
            const size_t pixelBytes = static_cast<size_t>(w) * static_cast<size_t>(h) * 4;

            // bgfx::copy allocates and copies the pixel data before Upload.
            const bgfx::Memory* mem = bgfx::copy(
                formatted->pixels, static_cast<uint32_t>(pixelBytes));

            bgfx::TextureHandle handle = bgfx::createTexture2D(
                static_cast<uint16_t>(w),
                static_cast<uint16_t>(h),
                false,  // no mipmaps
                1,      // one layer
                bgfx::TextureFormat::RGBA8,
                BGFX_TEXTURE_NONE |
                BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
                mem);

            {
                std::lock_guard<std::mutex> lock(textTextureCacheMutex);
                textTextureCache[text] = { handle, currentTime, pixelBytes, w, h };
                totalCacheMemory += pixelBytes;
            }

            cachedHandle = handle;
            textureWidth = w;
            textureHeight = h;

            if (formatted != surface) SDL_FreeSurface(formatted);
            SDL_FreeSurface(surface);
        }

        // ── Dispatch draw (alpha blending handled inside shader Submit state) ─────
        const glm::vec2 drawSize(scale.x * static_cast<float>(textureWidth),
            scale.y * static_cast<float>(textureHeight));

        if (shader.empty())
            DrawTexturedRect(pos, drawSize, rotation, pivot, cachedHandle, color);
        else
            DrawTexturedRectShader(pos, drawSize, rotation, pivot, cachedHandle, color, shader);
    }

    // ── MaintainCache ─────────────────────────────────────────────────────────────

    static void MaintainCache()
    {
        std::lock_guard<std::mutex> lock(textTextureCacheMutex);

        // Evict stale entries
        const float now = currentTime;
        for (auto it = textTextureCache.begin(); it != textTextureCache.end(); )
        {
            if (now - it->second.lastUsedTime > MAX_UNUSED_SECONDS)
            {
                if (bgfx::isValid(it->second.textureHandle))
                    bgfx::destroy(it->second.textureHandle);
                totalCacheMemory -= it->second.memorySize;
                it = textTextureCache.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Evict LRU entries until we're back under the memory budget
        while (totalCacheMemory > MAX_CACHE_MEMORY && !textTextureCache.empty())
        {
            auto lruIt = std::min_element(
                textTextureCache.begin(), textTextureCache.end(),
                [](const auto& a, const auto& b) {
                    return a.second.lastUsedTime < b.second.lastUsedTime;
                });

            if (bgfx::isValid(lruIt->second.textureHandle))
                bgfx::destroy(lruIt->second.textureHandle);
            totalCacheMemory -= lruIt->second.memorySize;
            textTextureCache.erase(lruIt);
        }
    }

    // ── EndFrame ──────────────────────────────────────────────────────────────────

    void EndFrame()
    {
        currentTime = Time::GameTimeNoPause;
        MaintainCache();
    }

} // namespace UiRenderer