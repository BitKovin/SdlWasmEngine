#include "ShaderManager.h"

#include <FileSystem/FileSystem.h>

#include <Renderer/Abstractions/ViewIdManager.h>
#include <BgfxStateManager.h>

void ShaderManager::SavePSOsToFile(const std::string& filePath)
{
    json root;
    root["psos"] = json::array();

    for (const auto& pso : psoList)
    {
        json entry;
        entry["renderState"] = pso.key.renderState;
        entry["vertexShader"] = pso.vertexShaderName;
        entry["pixelShader"] = pso.pixelShaderName;

        root["psos"].push_back(entry);
    }

    std::string output = root.dump(4); // pretty print

    FileSystemEngine::WriteFile(filePath, output);

    Logger::Log("Saved PSOs: " + std::to_string(psoList.size()));
}

void ShaderManager::CompilePSOsFromFile(const std::string& filePath)
{

    auto startState = BgfxStateManager::GetState();

    std::string data = FileSystemEngine::ReadFile(filePath);

    if (data.empty())
    {
        Logger::Log("PSO file empty or missing: " + filePath);
        return;
    }

    json root;

    try
    {
        root = json::parse(data);
    }
    catch (const std::exception& e)
    {
        Logger::Log(std::string("Failed to parse PSO JSON: ") + e.what());
        return;
    }

    if (!root.contains("psos") || !root["psos"].is_array())
    {
        Logger::Log("Invalid PSO JSON format");
        return;
    }

    //  LOCAL container only
    std::vector<PsoState> loadedPSOs;

    for (const auto& entry : root["psos"])
    {
        if (!entry.contains("renderState") ||
            !entry.contains("vertexShader") ||
            !entry.contains("pixelShader"))
        {
            continue;
        }

        PsoState state;

        state.key.renderState = entry["renderState"].get<uint64_t>();
        state.vertexShaderName = entry["vertexShader"].get<std::string>();
        state.pixelShaderName = entry["pixelShader"].get<std::string>();

        loadedPSOs.push_back(state);
    }

    Logger::Log("Loaded PSOs from file: " + std::to_string(loadedPSOs.size()));



    for (const auto& pso : loadedPSOs)
    {


        Shader* program = GetShaderProgram(
            pso.vertexShaderName,
            pso.pixelShaderName
        );

        const bgfx::VertexLayout layout = VertexData::Declaration();

        // Allocate transient buffers (per-frame safe)
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer  tib;

        const uint32_t numVertices = 3;
        const uint32_t numIndices = 3;

        if (!bgfx::getAvailTransientVertexBuffer(numVertices, layout) ||
            !bgfx::getAvailTransientIndexBuffer(numIndices))
        {
            Logger::Log("Not enough transient buffer space for PSO warmup");
            continue;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, layout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        // 🔹 Fill vertex data (ZERO is fine)
        VertexData* verts = reinterpret_cast<VertexData*>(tvb.data);

        verts[0] = {};
        verts[1] = {};
        verts[2] = {};

        // Minimal valid triangle positions (important!)
        verts[0].Position = { 0.0f, 0.0f, 0.0f };
        verts[1].Position = { 1.0f, 0.0f, 0.0f };
        verts[2].Position = { 0.0f, 1.0f, 0.0f };

        // 🔹 Fill indices
        uint16_t* indices = reinterpret_cast<uint16_t*>(tib.data);
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;

        // 🔹 Bind buffers
        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);

        // 🔹 Optional: instance buffer (safe fallback = 1 instance)
        bgfx::InstanceDataBuffer idb;
        InstanceData* inst = InstanceData::Alloc(idb, 1);

        inst[0] = {};
        inst[0].model[0] = { 1,0,0,0 };
        inst[0].model[1] = { 0,1,0,0 };
        inst[0].model[2] = { 0,0,1,0 };
        inst[0].model[3] = { 0,0,0,1 };
        inst[0].Color = { 1,1,1,1 };

        bgfx::setInstanceDataBuffer(&idb);

        // 🔹 Set state AFTER buffers (safer)
        bgfx::setState(pso.key.renderState);

        BgfxStateManager::SetState(pso.key.renderState);

        // 🔹 Submit
        program->Submit(ViewIdManager::GetCurrentId());

    }

    BgfxStateManager::SetState(startState);

}
