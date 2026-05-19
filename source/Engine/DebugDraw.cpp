#include "DebugDraw.hpp"

#include <cstdio>
#include <cstring>
#include <bgfx/bgfx.h>
#include <ShaderManager.h>
#include <Camera.h>
#include <Logger.hpp>

#include <Renderer/Abstractions/ViewIdManager.h>

// ─── Static member definitions ────────────────────────────────────────────────

std::mutex                                         DebugDraw::mainLock;
std::vector<std::unique_ptr<DebugLineCommand>>     DebugDraw::commands;
std::vector<DebugLineCommand*>                     DebugDraw::finalizedCommands;
bgfx::ProgramHandle                                DebugDraw::program     = BGFX_INVALID_HANDLE;
bgfx::ViewId                                       DebugDraw::viewId      = 0;
bool                                               DebugDraw::initialized = false;
bgfx::VertexLayout                                 DebugDraw::DebugVertex::layout;

bgfx::DynamicVertexBufferHandle DebugDraw::vertexBuffer = BGFX_INVALID_HANDLE;
uint32_t                        DebugDraw::vertexCapacity = 0;

// ─── DebugVertex ─────────────────────────────────────────────────────────────

void DebugDraw::DebugVertex::init()
{
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true /* normalised */)
        .end();

    if (!bgfx::isValid(vertexBuffer))
    {
        vertexCapacity = 256; // reasonable starting size
        vertexBuffer = bgfx::createDynamicVertexBuffer(
            vertexCapacity,
            DebugVertex::layout,
            BGFX_BUFFER_ALLOW_RESIZE);
    }

}



// ─── Renderer-type → shader subdirectory ─────────────────────────────────────

static const char* shaderSubdir()
{
    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12: return "dx11";
    case bgfx::RendererType::Metal:      return "metal";
    case bgfx::RendererType::Vulkan:     return "spirv";
    case bgfx::RendererType::OpenGLES:   return "essl";
    default:                             return "glsl";
    }
}

// ─── Init / Shutdown ─────────────────────────────────────────────────────────

void DebugDraw::Init()
{
    if (initialized) return;

    DebugVertex::init();

    program = ShaderManager::GetShaderProgram("debugDraw/vs_debugdraw", "debugDraw/fs_debugdraw")->GetProgram();
    initialized = bgfx::isValid(program);

    if (!initialized)
        Logger::Error("[DebugDraw] Failed to load shaders – drawing disabled.");
}

void DebugDraw::Shutdown()
{
    ClearCommands();
    if (bgfx::isValid(program))
    {
        bgfx::destroy(program);
        program = BGFX_INVALID_HANDLE;
    }
    initialized = false;
}

// ─── Command management ───────────────────────────────────────────────────────

void DebugDraw::ClearCommands()
{
    std::lock_guard<std::mutex> lock(mainLock);
    commands.clear();
    finalizedCommands.clear();
}

void DebugDraw::Line(vec3 start, vec3 end,
                     float duration, float /*thickness*/, uint32_t color)
{
    std::lock_guard<std::mutex> lock(mainLock);
    commands.push_back(
        std::make_unique<DebugLineCommand>(start, end, color, static_cast<double>(duration)));
}

void DebugDraw::Point(vec3 position, float duration, float radius, uint32_t color)
{

	std::vector<vec3> cubeVertexPositions = {
		{ -1, -1, -1 },
		{ 1, -1, -1 },
		{ 1, 1, -1 },
		{ -1, 1, -1 },
		{ -1, -1, 1 },
		{ 1, -1, 1 },
		{ 1, 1, 1 },
		{ -1, 1, 1 }
	};

	std::vector<vec3> unitDirections = {
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, -1, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 }
	};

    for (auto& vertex : cubeVertexPositions)
    {
        Line(position, position + vertex * radius, duration, 0.01f, color);
    }
    for (auto& vertex : unitDirections)
    {
        Line(position, position + vertex * radius, duration, 0.01f, color);
    }
    


}

void DebugDraw::Bounds(vec3 min, vec3 max,
                       float duration, float thickness, uint32_t color)
{
    const vec3 p000 = { min.x, min.y, min.z };
    const vec3 p001 = { min.x, min.y, max.z };
    const vec3 p010 = { min.x, max.y, min.z };
    const vec3 p011 = { min.x, max.y, max.z };
    const vec3 p100 = { max.x, min.y, min.z };
    const vec3 p101 = { max.x, min.y, max.z };
    const vec3 p110 = { max.x, max.y, min.z };
    const vec3 p111 = { max.x, max.y, max.z };

    // Bottom face
    Line(p000, p100, duration, thickness, color);
    Line(p100, p101, duration, thickness, color);
    Line(p101, p001, duration, thickness, color);
    Line(p001, p000, duration, thickness, color);

    // Top face
    Line(p010, p110, duration, thickness, color);
    Line(p110, p111, duration, thickness, color);
    Line(p111, p011, duration, thickness, color);
    Line(p011, p010, duration, thickness, color);

    // Vertical edges
    Line(p000, p010, duration, thickness, color);
    Line(p100, p110, duration, thickness, color);
    Line(p101, p111, duration, thickness, color);
    Line(p001, p011, duration, thickness, color);
}

void DebugDraw::Path(vector<vec3> path,
                     float duration, float thickness, uint32_t color)
{
    for (int i = 1; i < static_cast<int>(path.size()); ++i)
        Line(path[i - 1], path[i], duration, thickness, color);
}

void DebugDraw::IndexedMesh(std::vector<vec3> vertices,
    std::vector<uint32_t> indices,
    float duration, float thickness, uint32_t color)
{
    if (indices.size() < 3) return;

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const vec3& a = vertices[indices[i + 0]];
        const vec3& b = vertices[indices[i + 1]];
        const vec3& c = vertices[indices[i + 2]];

        Line(a, b, duration, thickness, color);
        Line(b, c, duration, thickness, color);
        Line(c, a, duration, thickness, color);
    }
}

// ─── Frame lifecycle ──────────────────────────────────────────────────────────

void DebugDraw::Finalize()
{
    std::lock_guard<std::mutex> lock(mainLock);

    finalizedCommands.clear();

    // Erase commands whose timer has expired.
    auto newEnd = std::remove_if(commands.begin(), commands.end(),
        [](const std::unique_ptr<DebugLineCommand>& cmd)
        {
            return !(cmd && cmd->drawTime.Wait());
        });
    commands.erase(newEnd, commands.end());

#ifndef DISTRIBUTION

    // Snapshot raw (non-owning) pointers for the render thread.
    finalizedCommands.reserve(commands.size());
    for (const auto& cmd : commands)
        finalizedCommands.push_back(cmd.get());   

#else
    commands.clear();
    finalizedCommands.clear();
#endif

}

void DebugDraw::Draw()
{
    Init();

    viewId = ViewIdManager::GetCurrentId();

    bgfx::setViewTransform(viewId, &Camera::finalizedView, &Camera::finalizedProjection);

    if (!initialized || finalizedCommands.empty())
        return;

    const uint32_t totalVertices = static_cast<uint32_t>(finalizedCommands.size()) * 2u;
    if (totalVertices == 0)
        return;

    // Ensure buffer is large enough
    if (!bgfx::isValid(vertexBuffer) || totalVertices > vertexCapacity)
    {
        if (bgfx::isValid(vertexBuffer))
        {
            bgfx::destroy(vertexBuffer);
            vertexBuffer = BGFX_INVALID_HANDLE;
        }

        vertexCapacity = std::max(totalVertices, 256u);

        vertexBuffer = bgfx::createDynamicVertexBuffer(
            vertexCapacity,
            DebugVertex::layout,
            BGFX_BUFFER_ALLOW_RESIZE);

        if (!bgfx::isValid(vertexBuffer))
            return;
    }

    // Build vertex data
    std::vector<DebugVertex> vertices(totalVertices);

    DebugVertex* out = vertices.data();
    for (const DebugLineCommand* c : finalizedCommands)
    {
        *out++ = { c->start.x, c->start.y, c->start.z, c->color };
        *out++ = { c->end.x,   c->end.y,   c->end.z,   c->color };
    }

    // Upload to GPU
    const bgfx::Memory* mem = bgfx::copy(
        vertices.data(),
        uint32_t(vertices.size() * sizeof(DebugVertex)));

    bgfx::update(vertexBuffer, 0, mem);

    // Bind + draw
    bgfx::setVertexBuffer(0, vertexBuffer, 0, totalVertices);

    bgfx::setState(
        BGFX_STATE_WRITE_RGB |
        BGFX_STATE_WRITE_A |
        BGFX_STATE_DEPTH_TEST_LESS |
        BGFX_STATE_PT_LINES);

    bgfx::submit(viewId, program);
}