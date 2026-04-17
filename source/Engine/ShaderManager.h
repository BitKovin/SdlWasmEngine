#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "Shader.hpp"
#include "AssetRegistry.h"

class ShaderManager
{
private:

    // =========================
    // 🔹 PROGRAM CACHE
    // =========================

    struct ProgramEntry
    {
        Shader* program;
        uint32_t index;
    };

    static inline std::unordered_map<std::string, ProgramEntry> programCache;
    static inline std::vector<ProgramEntry> programList;
    static inline uint32_t nextProgramIndex = 0;


    // =========================
    // 🔹 PSO SYSTEM
    // =========================

    struct PsoKey
    {
        uint64_t renderState;
        uint32_t programIndex;

        bool operator==(const PsoKey& other) const
        {
            return renderState == other.renderState &&
                programIndex == other.programIndex;
        }
    };

    struct PsoKeyHash
    {
        size_t operator()(const PsoKey& k) const
        {
            return std::hash<uint64_t>()(k.renderState) ^
                (std::hash<uint32_t>()(k.programIndex) << 1);
        }
    };

public:

    struct PsoState
    {
        PsoKey key;

        std::string vertexShaderName;
        std::string pixelShaderName;
    };

private:

    static inline std::unordered_map<PsoKey, PsoState, PsoKeyHash> psoCache;
    static inline std::vector<PsoState> psoList;

    static inline std::string NormalizeShaderName(const std::string& path)
    {
        std::string result = path;

        const std::string prefix = "GameData/shaders/source/";
        const std::string extension = ".sc";

        // 🔹 Remove prefix
        if (result.rfind(prefix, 0) == 0)
        {
            result = result.substr(prefix.length());
        }

        // 🔹 Remove extension
        if (result.size() >= extension.size() &&
            result.substr(result.size() - extension.size()) == extension)
        {
            result = result.substr(0, result.size() - extension.size());
        }

        return result;
    }

public:

    // =========================
    // 🔹 PROGRAM ACCESS
    // =========================

    static Shader* GetShaderProgram(
        const std::string& vertexShaderName = "default_vertex",
        const std::string& pixelShaderName = "default_pixel")
    {
        std::string key = vertexShaderName + "|" + pixelShaderName;

        auto it = programCache.find(key);
        if (it != programCache.end())
        {
            return it->second.program;
        }

        Logger::Log("Creating ShaderProgram: " + key);

        Shader* program = Shader::FromFiles(
            vertexShaderName.c_str(),
            pixelShaderName.c_str()
        );

        ProgramEntry entry;
        entry.program = program;
        entry.index = nextProgramIndex++;

        programCache[key] = entry;
        programList.push_back(entry);

        return program;
    }


    // =========================
    // 🔹 PROGRAM INDEX
    // =========================

    static uint32_t GetProgramIndex(
        const std::string& vertexShaderName,
        const std::string& pixelShaderName)
    {
        std::string key = vertexShaderName + "|" + pixelShaderName;

        auto it = programCache.find(key);
        if (it != programCache.end())
        {
            return it->second.index;
        }

        // Ensure program exists
        Shader* program = GetShaderProgram(vertexShaderName, pixelShaderName);

        // Now it must exist
        return programCache[key].index;
    }


    // =========================
    // 🔹 PSO REGISTRATION
    // =========================

    static void RegisterPSO(
        uint64_t renderState,
        std::string vertexShaderName,
        std::string pixelShaderName)
    {

        vertexShaderName = NormalizeShaderName(vertexShaderName);
        pixelShaderName = NormalizeShaderName(pixelShaderName);

        uint32_t programIndex = GetProgramIndex(vertexShaderName, pixelShaderName);

        PsoKey key{ renderState, programIndex };

        auto it = psoCache.find(key);
        if (it != psoCache.end())
        {
            return; // already registered
        }

        PsoState state;
        state.key = key;
        state.vertexShaderName = vertexShaderName;
        state.pixelShaderName = pixelShaderName;

        psoCache[key] = state;
        psoList.push_back(state);

        Logger::Log("Registered new PSO: " + vertexShaderName + " | " + pixelShaderName + " | " + to_string(renderState));
    }


    // =========================
    //  ACCESS FOR PRECOMPILE
    // =========================

    static const std::vector<PsoState>& GetAllPSOStates()
    {
        return psoList;
    }

    static void SavePSOsToFile(const std::string& filePath);

	static void CompilePSOsFromFile(const std::string& filePath);

    // =========================
    //  OPTIONAL: CLEAR
    // =========================

    static void ClearPSOCache()
    {
        psoCache.clear();
        psoList.clear();
    }
};