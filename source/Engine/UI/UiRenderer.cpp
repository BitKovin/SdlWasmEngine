#include "UiRenderer.h"
#include "../gl.h"
#include "../ShaderManager.h"
#include "../Camera.h"
#include <unordered_map>
#include <SDL2/SDL.h>
#include <iostream>
#include "../Time.hpp"
#include <mutex>
#include "UiManager.h"

// Cache entry structure
struct TextureCacheEntry {
    GLuint textureID;      // OpenGL texture ID
    float lastUsedTime;    // Last time used (seconds)
    size_t memorySize;     // Memory size in bytes
    int width;             // Texture width for rendering
    int height;            // Texture height for rendering
};

// Static variables for renderer state and cache
static GLuint quadVAO = 0;
static GLuint quadVBO = 0;
static ShaderProgram* texturedShader = nullptr;
static ShaderProgram* flatColorShader = nullptr;
static std::unordered_map<std::string, TextureCacheEntry> textTextureCache;
static size_t totalCacheMemory = 0;                      // Total memory used by the cache
static const size_t MAX_CACHE_MEMORY = 50 * 1024 * 1024; // 50 MB limit
static const float MAX_UNUSED_SECONDS = 2.0f;           // Evict textures unused for 10 seconds
static float currentTime = 0.0f;                         // Current time in seconds

static std::mutex textTextureCacheMutex;

namespace UiRenderer {

    void Init() {
        float quadVertices[] = {
            // pos      // uv
            0.0f, 1.0f,  0.0f, 1.0f,
            1.0f, 0.0f,  1.0f, 0.0f,
            0.0f, 0.0f,  0.0f, 0.0f,
            0.0f, 1.0f,  0.0f, 1.0f,
            1.0f, 1.0f,  1.0f, 1.0f,
            1.0f, 0.0f,  1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1); // texcoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);

        texturedShader = ShaderManager::GetShaderProgram("ui", "ui_textured");
        flatColorShader = ShaderManager::GetShaderProgram("ui", "ui_flatcolor");
    }

    void Shutdown() {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        delete texturedShader;
        delete flatColorShader;

        // Clean up cached textures
        for (auto& pair : textTextureCache) {
            glDeleteTextures(1, &pair.second.textureID);
        }
        textTextureCache.clear();
        totalCacheMemory = 0;
    }

    void SetShaderProjection(ShaderProgram* shader) {
        // use floats — avoid integer truncation
        float screenHeight = static_cast<float>(UiManager::GetScaledUiHeight()); // pixels
        float screenWidth = screenHeight * Camera::AspectRatio; // pixels (float!)

        if(customViewport)
        {
            screenWidth = static_cast<float>(customViewportSize.x);
            screenHeight = static_cast<float>(customViewportSize.y);
		}

        // orthographic projection with top-left origin (y down)
        glm::mat4 uiProjection = glm::ortho(
            0.0f,
            screenWidth,
            screenHeight,
            0.0f,
            -1.0f,
            1.0f
        );

        shader->SetUniform("u_Projection", uiProjection);

    }

    void DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, GLuint texture, const glm::vec4& color) {
        texturedShader->UseProgram();

        SetShaderProjection(texturedShader);

        glm::vec2 pivotLocal = pivot * size;

        glm::mat4 model(1.0f);

        // place top-left of image
        model = glm::translate(model, glm::vec3(pos, 0.0f));

        // move pivot to origin
        model = glm::translate(model, glm::vec3(pivotLocal, 0.0f));

        // rotate around pivot
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, 1));

        // move pivot back
        model = glm::translate(model, glm::vec3(-pivotLocal, 0.0f));

        // scale quad to size
        model = glm::scale(model, glm::vec3(size, 1.0f));

        texturedShader->SetUniform("u_Model", model);

        texturedShader->SetUniform("u_Color", color);

        texturedShader->SetTexture("u_Texture", texture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void DrawTexturedRectShader(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, GLuint texture, const glm::vec4& color, const string& shader)
    {
        auto shaderProgram = ShaderManager::GetShaderProgram("ui", shader); 
        shaderProgram->UseProgram();

        SetShaderProjection(shaderProgram);

        glm::mat4 model(1.0f);

        // 1. Final already-pivoted element position
        model = glm::translate(model, glm::vec3(pos, 0.0f));

        // 2. Pivot offset inside local space (needed for rotation)
        glm::vec2 pivotOffset = pivot * size;

        // 3. Move pivot → origin
        model = glm::translate(model, glm::vec3(pivotOffset, 0.0f));

        // 4. Apply rotation
        model = glm::rotate(model, MathHelper::ToRadians(rotation), glm::vec3(0, 0, 1));

        // 5. Move back after rotation
        model = glm::translate(model, glm::vec3(-pivotOffset, 0.0f));

        // 6. Apply scale
        model = glm::scale(model, glm::vec3(size, 1.0f));
        
        shaderProgram->SetUniform("u_Model", model);
        shaderProgram->SetUniform("u_Color", color);

        shaderProgram->SetTexture("u_Texture", texture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void DrawTexturedRectShaderParams(const glm::vec2& pos, const glm::vec2& size, float rotation, vec2 pivot, std::unordered_map<std::string, GLuint>& textures, std::unordered_map<std::string, float>& scalars, const glm::vec4& color, const string& shader)
    {
        auto shaderProgram = ShaderManager::GetShaderProgram("ui", shader);
        shaderProgram->UseProgram();

        SetShaderProjection(shaderProgram);

        glm::mat4 model(1.0f);

        // 1. Final already-pivoted element position
        model = glm::translate(model, glm::vec3(pos, 0.0f));

        // 2. Pivot offset inside local space (needed for rotation)
        glm::vec2 pivotOffset = pivot * size;

        // 3. Move pivot → origin
        model = glm::translate(model, glm::vec3(pivotOffset, 0.0f));

        // 4. Apply rotation
        model = glm::rotate(model, MathHelper::ToRadians(rotation), glm::vec3(0, 0, 1));

        // 5. Move back after rotation
        model = glm::translate(model, glm::vec3(-pivotOffset, 0.0f));

        // 6. Apply scale
        model = glm::scale(model, glm::vec3(size, 1.0f));

        shaderProgram->SetUniform("u_Model", model);
        shaderProgram->SetUniform("u_Color", color);

        for (auto& tex : textures)
        {
            shaderProgram->SetTexture(tex.first, tex.second);
        }

        for (auto& scalar : scalars)
        {
            shaderProgram->SetUniform(scalar.first, scalar.second);
        }
        

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
#ifndef GL_ES_PROFILE
        flatColorShader->UseProgram();
        SetShaderProjection(flatColorShader);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));
        flatColorShader->SetUniform("u_Model", model);
        flatColorShader->SetUniform("u_Color", color);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif // !GL_ES_PROFILE
    }

    void DrawText(std::string text, TTF_Font* font, const glm::vec2& pos, float rotation, glm::vec2 pivot,
        const glm::vec4& color, const glm::vec2& scale, const std::string& shader) {
        if (!font) {
            std::cerr << "No font provided for DrawText." << std::endl;
            return;
        }

        if (text.empty())return;

        GLuint textureID = 0;
        int textureWidth = 0;
        int textureHeight = 0;

        // Lock cache for read/update
        {
            std::lock_guard<std::mutex> lock(textTextureCacheMutex);
            auto it = textTextureCache.find(text);
            if (it != textTextureCache.end()) {
                textureID = it->second.textureID;
                textureWidth = it->second.width;
                textureHeight = it->second.height;
                it->second.lastUsedTime = currentTime; // update usage
            }
        }

        if (textureID == 0) {
            // Convert color from [0.0,1.0] to [0,255]
            SDL_Color sdlColor = {
                static_cast<Uint8>(glm::clamp(1.0f, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(1.0f, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(1.0f, 0.0f, 1.0f) * 255.0f),
                static_cast<Uint8>(glm::clamp(1.0f, 0.0f, 1.0f) * 255.0f)
            };

            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
            if (!surface) {
                std::cerr << "TTF_RenderUTF8_Blended Error: " << TTF_GetError() << std::endl;
                return;
            }

            // (Optional) Convert surface to a well-known pixel format to avoid format surprises.
            // SDL_PIXELFORMAT_RGBA32 is usually safe for uploading as GL_RGBA + GL_UNSIGNED_BYTE.
            SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
            if (!formatted) {
                // fallback to original surface
                formatted = surface;
            }

            size_t textureMemory = static_cast<size_t>(formatted->w) * static_cast<size_t>(formatted->h) * 4;

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            // no need to set UNPACK_ROW_LENGTH here if using contiguous formatted->pixels
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // We converted to RGBA32 -> use GL_RGBA
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formatted->w, formatted->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formatted->pixels);

            // cache the texture metadata
            {
                std::lock_guard<std::mutex> lock(textTextureCacheMutex);
                textTextureCache[text] = { textureID, currentTime, textureMemory, formatted->w, formatted->h };
                totalCacheMemory += textureMemory;
            }

            textureWidth = formatted->w;
            textureHeight = formatted->h;

            if (formatted != surface) SDL_FreeSurface(formatted);
            SDL_FreeSurface(surface);
        }

        // Enable blending for alpha support
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::vec2 drawSize(scale.x * textureWidth, scale.y * textureHeight);

        if (shader.empty()) {
            DrawTexturedRect(pos, drawSize, rotation, pivot, textureID, color);
        }
        else {
            DrawTexturedRectShader(pos, drawSize, rotation, pivot, textureID, color, shader);
        }
    }


    void MaintainCache() {
        std::lock_guard<std::mutex> lock(textTextureCacheMutex);

        // First, remove any textures that are too old
        auto now = currentTime;
        for (auto it = textTextureCache.begin(); it != textTextureCache.end(); ) {
            if (now - it->second.lastUsedTime > MAX_UNUSED_SECONDS) {
                GLuint texToDelete = it->second.textureID;
                size_t mem = it->second.memorySize;
                it = textTextureCache.erase(it);
                glDeleteTextures(1, &texToDelete);
                totalCacheMemory -= mem;
            }
            else {
                ++it;
            }
        }

        // If cache is still over limit, remove least recently used textures
        while (totalCacheMemory > MAX_CACHE_MEMORY && !textTextureCache.empty()) {
            // Find the least recently used element
            auto lruIt = std::min_element(
                textTextureCache.begin(), textTextureCache.end(),
                [](const auto& a, const auto& b) {
                    return a.second.lastUsedTime < b.second.lastUsedTime;
                }
            );

            GLuint texToDelete = lruIt->second.textureID;
            size_t mem = lruIt->second.memorySize;
            glDeleteTextures(1, &texToDelete);
            totalCacheMemory -= mem;
            textTextureCache.erase(lruIt);
        }
    }



    void EndFrame() {
        // Update current time (SDL_GetTicks returns milliseconds, convert to seconds)
        currentTime = Time::GameTimeNoPause;
        MaintainCache();
    }

} // namespace UiRenderer