#pragma once

#include <mutex>
#include <vector>
#include <memory>

#include "StaticMesh.hpp"
#include "glm.h"
#include "gl.h"
#include "Delay.hpp"

class DebugDrawCommand
{
public:
    DebugDrawCommand() {}
    virtual ~DebugDrawCommand() {}

    Delay DrawTime = Delay(0.2);

    virtual void Draw() {}
};

class DrawCommandLine : public DebugDrawCommand
{
    vec3 start;
    vec3 end;
    float thickness;
    StaticMesh lineMesh;


public:
    DrawCommandLine(vec3 s, vec3 e, float th, double duration)
    {
        start = s;
        end = e;
        thickness = th;

        DrawTime = Delay(duration);

        lineMesh.LoadFromFile("GameData/cube.obj");
        lineMesh.SetPixelShader("solidRed_pixel");

        lineMesh.Position = mix(s, e, 0.5);
        lineMesh.Rotation = MathHelper::FindLookAtRotation(s, e);

        lineMesh.Scale.z = distance(s, e);

        lineMesh.Scale.x = th;
        lineMesh.Scale.y = th;

        lineMesh.FinalizeFrameData();

    }

    void Draw() override
    {
        
        lineMesh.DrawForward(Camera::finalizedView, Camera::finalizedProjection);
    }
};

class DebugDraw
{
private:
    static std::mutex mainLock;

    // This container owns the commands.
    static std::vector<std::unique_ptr<DebugDrawCommand>> commands;
    // This container is a snapshot for the render thread; it does not own the objects.
    static std::vector<DebugDrawCommand*> finalizedCommands;

    static void AddCommand(std::unique_ptr<DebugDrawCommand> command)
    {
        std::lock_guard<std::mutex> lock(mainLock);
        commands.push_back(std::move(command));
    }

public:
    // Called from any thread to add a draw command.
    static void Line(vec3 start, vec3 end, float duration = 0.1f, float thickness = 0.02f)
    {
        AddCommand(std::make_unique<DrawCommandLine>(start, end, thickness, duration));
    }

    static void Bounds(vec3 min, vec3 max, float duration = 0.1f, float thickness = 0.02f)
    {
        vec3 p000 = { min.x, min.y, min.z };
        vec3 p001 = { min.x, min.y, max.z };
        vec3 p010 = { min.x, max.y, min.z };
        vec3 p011 = { min.x, max.y, max.z };
        vec3 p100 = { max.x, min.y, min.z };
        vec3 p101 = { max.x, min.y, max.z };
        vec3 p110 = { max.x, max.y, min.z };
        vec3 p111 = { max.x, max.y, max.z };

        // Bottom face
        Line(p000, p100, duration, thickness);
        Line(p100, p101, duration, thickness);
        Line(p101, p001, duration, thickness);
        Line(p001, p000, duration, thickness);

        // Top face
        Line(p010, p110, duration, thickness);
        Line(p110, p111, duration, thickness);
        Line(p111, p011, duration, thickness);
        Line(p011, p010, duration, thickness);

        // Vertical edges
        Line(p000, p010, duration, thickness);
        Line(p100, p110, duration, thickness);
        Line(p101, p111, duration, thickness);
        Line(p001, p011, duration, thickness);
    }

    static void Path(vector<vec3> path, float duration = 1, float thickness = 0.02f)
    {
        for (int i = 1; i < path.size(); i++)
        {
            Line(path[i - 1], path[i], duration, thickness);
        }
    }

    // This method is called to produce a snapshot for the render thread. It:
    // 1. Removes expired commands from 'commands'
    // 2. Copies pointers to the remaining commands into 'finalizedCommands'
    static void Finalize()
    {
        std::lock_guard<std::mutex> lock(mainLock);

        // Clear the previous snapshot.
        finalizedCommands.clear();

        // Remove expired commands
        auto new_end = std::remove_if(commands.begin(), commands.end(),
            [](const std::unique_ptr<DebugDrawCommand>& cmd) {
                return !(cmd && cmd->DrawTime.Wait());
            });
        commands.erase(new_end, commands.end());

        // Now collect raw pointers to active commands
        for (const auto& cmd : commands) {
            finalizedCommands.push_back(cmd.get());
        }
    }

    // Called by the render thread.
    static void Draw()
    {
        // It is assumed that Finalize() is called before Draw() and that
        // finalizedCommands remains consistent for the duration of rendering.
        for (auto& command : finalizedCommands)
        {
            command->Draw();
        }
    }
};