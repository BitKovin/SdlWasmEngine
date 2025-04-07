#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "Shader.hpp"
#include "skinned_model.hpp"
#include "model.hpp"
#include "Texture.hpp"
#include "Logger.hpp"

class AssetRegistry
{

private:
    static std::unordered_map<std::string, Shader*> shaderCache;
    static std::unordered_map<std::string, Texture*> textureCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelCache;

    static std::unordered_map<std::string, TTF_Font*> fontCache;

public:
	
    static Shader* GetShaderByName(const std::string& name, ShaderType shaderType)
    {

        string fileEnding;

        switch (shaderType)
        {

        case(ShaderType::PixelShader):
                fileEnding = ".frag";
                break;
        case(ShaderType::VertexShader):
            fileEnding = ".vert";
            break;

        default:
            break;
        }


        string fileName = name + fileEnding;


        std::string key = fileName; // Unique key for each shader type

        // Check if the shader is already cached
        auto it = shaderCache.find(key);
        if (it != shaderCache.end())
        {
            return it->second; // Return cached shader
        }

        std::string filePath = "GameData/Shaders/" + fileName;
        std::string shaderCode = ReadFileToString(filePath);

        // Cache the newly loaded shader
        shaderCache[key] = Shader::FromCode(shaderCode.c_str(), shaderType);

        return shaderCache[key];
    }

    static Texture* GetTextureFromFile(const char* filename)
    {

        string key = filename;

        auto it = textureCache.find(key);
        if (it != textureCache.end())
        {
            return it->second; // Return cached shader
        }

        textureCache[key] = new Texture(filename, true);

        return textureCache[key];
    }

    static TTF_Font* GetFontFromFile(const char* filename, int fontSize) {
        std::string key = std::string(filename) + "_" + std::to_string(fontSize);
        auto it = fontCache.find(key);
        if (it != fontCache.end()) {
            return it->second; // Return cached font.
        }

        TTF_Font* font = TTF_OpenFont(filename, fontSize);
        if (!font) {
            std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
            return nullptr;
        }

        

        fontCache[key] = font;
        return font;
    }

    static std::string ReadFileToString(string filename) {
        SDL_RWops* file = SDL_RWFromFile(filename.c_str(), "r");
        if (!file) {
            Logger::Log("Failed to open file: " + filename);
            return "";
        }

        Sint64 fileSize = SDL_RWsize(file);
        if (fileSize < 0) {
            Logger::Log("Failed to get file size: " + filename);
            SDL_RWclose(file);
            return "";
        }

        std::string content(fileSize, '\0'); // Pre-allocate string space

        Sint64 bytesRead = SDL_RWread(file, &content[0], 1, fileSize);
        if (bytesRead != fileSize) {
            SDL_Log("Could not read the entire file.");
            Logger::Log("Could not read the entire file: " + filename);
            SDL_RWclose(file);
            return "";
        }

        SDL_RWclose(file);
        return content;
    }

    static roj::SkinnedModel* GetSkinnedModelFromFile(const string& path)
    {

        auto it = skinnedModelCache.find(path);
        if (it != skinnedModelCache.end())
        {
            return it->second; // Return cached shader
        }

        roj::ModelLoader<roj::SkinnedMesh> modelLoader;

        modelLoader.load(path);

        Logger::Log(modelLoader.getInfoLog());

        skinnedModelCache[path] = new roj::SkinnedModel(modelLoader.get());


        return skinnedModelCache[path];
    }

private:

};
