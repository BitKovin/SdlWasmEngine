#pragma once

#include "Shader.hpp"

#include "AssetRegisty.hpp"

class ShaderManager
{
private:
    static std::unordered_map<std::string, ShaderProgram*> shaderProgramCache;

public:
    static ShaderProgram* GetShaderProgram(const std::string& vertexShaderName = "default_vertex", const std::string& pixelShaderName = "default_pixel")
    {
        std::string key = vertexShaderName + "+" + pixelShaderName; // Unique key for shader program

        // Check if the program is already cached
        auto it = shaderProgramCache.find(key);
        if (it != shaderProgramCache.end())
        {
            return it->second; // Return cached program
        }

        // Load shaders
        Shader vertexShader = AssetRegistry::GetShaderByName(vertexShaderName, ShaderType::VertexShader);
        Shader pixelShader = AssetRegistry::GetShaderByName(pixelShaderName, ShaderType::PixelShader);

        // Create and link the shader program
        ShaderProgram* program = new ShaderProgram();
        program->AttachShader(vertexShader)->AttachShader(pixelShader)->LinkProgram();

        // Cache the created shader program
        shaderProgramCache[key] = program;

        return program;
    }
};

std::unordered_map<std::string, ShaderProgram*> ShaderManager::shaderProgramCache;