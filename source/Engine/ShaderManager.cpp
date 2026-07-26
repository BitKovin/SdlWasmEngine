#include "ShaderManager.h"

#include <FileSystem/FileSystem.h>

#include <Renderer/Abstractions/ViewIdManager.h>
#include <BgfxStateManager.h>

#include <EngineMain.h>

void ShaderManager::Shutdown()
{

    for (auto s : programCache)
    {
        delete s.second.program;
    }

    programCache.clear();
    Shader::ClearStaticResources();
}

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

    std::vector<PsoState> loadedPSOs;
    for (const auto& entry : root["psos"])
    {
        if (!entry.contains("renderState") ||
            !entry.contains("vertexShader") ||
            !entry.contains("pixelShader"))
            continue;

        PsoState state;
        state.key.renderState = entry["renderState"].get<uint64_t>();
        state.vertexShaderName = entry["vertexShader"].get<std::string>();
        state.pixelShaderName = entry["pixelShader"].get<std::string>();

        loadedPSOs.push_back(state);
    }

    Logger::Log("Loaded PSOs from file: " + std::to_string(loadedPSOs.size()));

    if (loadedPSOs.empty())
    {
        BgfxStateManager::SetState(startState);
        return;
    }

    // ===================================================================
    // Create tiny dummy framebuffers that cover ALL real-world variations
    // used in your Renderer (MSAA on/off + different depth formats)
    // ===================================================================
    std::vector<bgfx::FrameBufferHandle> dummyFBs;

    const uint16_t dummySize = 8;

    {
        auto validTex = [](bgfx::TextureFormat::Enum fmt, uint64_t flags)
            {
                return bgfx::isTextureValid(1, false, 1, fmt, flags);
            };

        // 1. RGBA16F + Depth24
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::RGBA16F, flags) &&
                validTex(bgfx::TextureFormat::D24, flags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA16F, flags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, flags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 2. RGBA8 + Depth24
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::RGBA8, flags) &&
                validTex(bgfx::TextureFormat::D24, flags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA8, flags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, flags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 3. RGB8 + Depth24
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::RGB8, flags) &&
                validTex(bgfx::TextureFormat::D24, flags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGB8, flags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, flags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 4. MSAA X2
        {
            uint64_t cflags = BGFX_TEXTURE_RT_MSAA_X2;
            uint64_t dflags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT_MSAA_X2;
            if (validTex(bgfx::TextureFormat::RGBA16F, cflags) &&
                validTex(bgfx::TextureFormat::D24, dflags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA16F, cflags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, dflags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 5. MSAA X4
        {
            uint64_t cflags = BGFX_TEXTURE_RT_MSAA_X4;
            uint64_t dflags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT_MSAA_X4;
            if (validTex(bgfx::TextureFormat::RGBA16F, cflags) &&
                validTex(bgfx::TextureFormat::D24, dflags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA16F, cflags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, dflags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 6. MSAA X8
        {
            uint64_t cflags = BGFX_TEXTURE_RT_MSAA_X8;
            uint64_t dflags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT_MSAA_X8;
            if (validTex(bgfx::TextureFormat::RGBA16F, cflags) &&
                validTex(bgfx::TextureFormat::D24, dflags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA16F, cflags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, dflags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 7. MSAA X16
        {
            uint64_t cflags = BGFX_TEXTURE_RT_MSAA_X16;
            uint64_t dflags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT_MSAA_X16;
            if (validTex(bgfx::TextureFormat::RGBA16F, cflags) &&
                validTex(bgfx::TextureFormat::D24, dflags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA16F, cflags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, dflags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }

        // 8. Depth-only D32F
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::D32F, flags))
            {
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D32F, flags);
                bgfx::Attachment att;
                att.init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(1, &att));
            }
        }

        // 9. Depth-only D24F
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::D24F, flags))
            {
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24F, flags);
                bgfx::Attachment att;
                att.init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(1, &att));
            }
        }

        // 10. RGBA8 + Depth24 final pass
        {
            uint64_t flags = BGFX_TEXTURE_RT;
            if (validTex(bgfx::TextureFormat::RGBA8, flags) &&
                validTex(bgfx::TextureFormat::D24, flags))
            {
                bgfx::TextureHandle color = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::RGBA8, flags);
                bgfx::TextureHandle depth = bgfx::createTexture2D(dummySize, dummySize, false, 1, bgfx::TextureFormat::D24, flags);
                bgfx::Attachment atts[2];
                atts[0].init(color);
                atts[1].init(depth);
                dummyFBs.push_back(bgfx::createFrameBuffer(2, atts));
            }
        }
    }

    // ===================================================================
    // Allocate one warmup view per FB variant — OUTSIDE the PSO loop.
    // This is the critical fix: view IDs = O(FBs), not O(PSOs x FBs),
    // so we never exceed BGFX_CONFIG_MAX_VIEWS (256).
    // ===================================================================
    std::vector<bgfx::ViewId> warmupViews;
    warmupViews.reserve(dummyFBs.size());

    for (bgfx::FrameBufferHandle dummyFB : dummyFBs)
    {
        bgfx::ViewId view = ViewIdManager::GiveNextId();
        bgfx::setViewFrameBuffer(view, dummyFB);
        bgfx::setViewRect(view, 0, 0, dummySize, dummySize);
        bgfx::setViewClear(view, BGFX_CLEAR_NONE);
        warmupViews.push_back(view);
    }

    // ===================================================================
    // Warmup loop — submit every PSO into each pre-configured view.
    // Views are reused across PSOs; no new IDs are minted here.
    // ===================================================================
    const bgfx::VertexLayout layout = VertexData::Declaration();

    for (const auto& pso : loadedPSOs)
    {
        Shader* program = GetShaderProgram(pso.vertexShaderName, pso.pixelShaderName);

        const bool isInstanced = (pso.vertexShaderName.find("instanced") != std::string::npos);

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;
        const uint32_t numVerts = 3;
        const uint32_t numInds = 3;

        if (!bgfx::getAvailTransientVertexBuffer(numVerts, layout) ||
            !bgfx::getAvailTransientIndexBuffer(numInds))
        {
            Logger::Log("Not enough transient buffer space for PSO warmup");
            continue;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVerts, layout);
        bgfx::allocTransientIndexBuffer(&tib, numInds);

        VertexData* verts = reinterpret_cast<VertexData*>(tvb.data);
        verts[0].Position = { 0.0f, 0.0f, 0.0f };
        verts[1].Position = { 1.0f, 0.0f, 0.0f };
        verts[2].Position = { 0.0f, 1.0f, 0.0f };

        uint16_t* indices = reinterpret_cast<uint16_t*>(tib.data);
        indices[0] = 0; indices[1] = 1; indices[2] = 2;

        for (bgfx::ViewId warmupView : warmupViews)
        {
            bgfx::setVertexBuffer(0, &tvb);
            bgfx::setIndexBuffer(&tib);

            if (isInstanced)
            {
                bgfx::InstanceDataBuffer idb;
                InstanceData* inst = InstanceData::Alloc(idb, 1);
                inst[0] = {};
                inst[0].model[0] = { 1,0,0,0 };
                inst[0].model[1] = { 0,1,0,0 };
                inst[0].model[2] = { 0,0,1,0 };
                inst[0].model[3] = { 0,0,0,1 };
                inst[0].Color = { 1,1,1,1 };
                bgfx::setInstanceDataBuffer(&idb);
            }

            BgfxStateManager::SetState(pso.key.renderState);
            BgfxStateManager::Apply();
            program->Submit(warmupView);
        }
    }

    // Restore state before frame() so the flip sees consistent state
    BgfxStateManager::SetState(startState);

    bgfx::frame();

    // Cleanup dummy framebuffers
    for (auto fb : dummyFBs)
        if (bgfx::isValid(fb))
            bgfx::destroy(fb);

    Logger::Log("PSO warmup completed for " + std::to_string(loadedPSOs.size()) +
        " shaders x " + std::to_string(dummyFBs.size()) + " framebuffer variants");

    BgfxStateManager::SetState(startState);
}
