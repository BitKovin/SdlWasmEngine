#pragma once
#include <SDL2/SDL_ttf.h>
#include <set>
#include "malloc_override.h"

#include "Shader.hpp"
#include "skinned_model.hpp"
#include "model.hpp"
#include "Texture.hpp"
#include "TextureCube.hpp"
#include "Logger.hpp"
#include "Video/Video.h"
#include <vector>

struct CachedFont {
    TTF_Font* font = nullptr;
    std::vector<uint8_t> data; // keeps font memory alive
};

class AssetRegistry
{

private:
    static std::unordered_map<std::string, Shader*> shaderCache;
    static std::unordered_map<std::string, Texture*> textureCache;
    static std::unordered_map<std::string, Video*> videoCache;
    static std::unordered_map<std::string, CubemapTexture*> textureCubeCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelAnimationCache;

    static std::set<std::string> loadedAssetsDuringLoading;

    static inline bool loadingLevel = false;

    static std::unordered_map<std::string, CachedFont> fontCache;



public:
	
    static void ClearMemory();
    static void ClearUnusedMemory();
    static bool IsAssetUsed(std::string filename);


    static void ReloadShaders();

    static Texture* GetTextureFromFile(string filename);

    static CubemapTexture* GetTextureCubeFromFile(string filename);

    static void RegisterTexture(Texture* texture, string path);

    static Video* GetVideoFromFile(string filename);

    static TTF_Font* GetFontFromFile(const char* filename, int fontSize);

    static std::string ReadFileToString(string filename);

    static void BeginLevelLoad();
    static void EndLevelLoad();

    static void MarkAsUsed(std::string filename);

    static roj::SkinnedModel* GetSkinnedModelFromFile(const string& path);
    static roj::SkinnedModel* GetSkinnedAnimationFromFile(const string& path);

private:

    static void MarkModelTexturesAsUsed(roj::SkinnedModel* model, std::string path);

};
