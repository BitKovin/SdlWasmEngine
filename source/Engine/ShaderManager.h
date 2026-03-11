#pragma once

#include "Shader.hpp"

#include "AssetRegistry.h"

class ShaderManager
{
private:
    static std::unordered_map<std::string, Shader*> shaderProgramCache;

public:
    static Shader* GetShaderProgram(const std::string& vertexShaderName = "default_vertex", const std::string& pixelShaderName = "default_pixel")
    {
        std::string key = vertexShaderName + pixelShaderName; // Unique key for shader program

        // Check if the program is already cached
        auto it = shaderProgramCache.find(key);
        if (it != shaderProgramCache.end())
        {
            return (it->second); // Return cached program
        }

		Logger::Log("Creating new ShaderProgram: " + key);

		Shader* program = Shader::FromFiles(vertexShaderName.c_str(), pixelShaderName.c_str());

		shaderProgramCache[key] = program; // Cache the newly created program

        return program;
    }
};
