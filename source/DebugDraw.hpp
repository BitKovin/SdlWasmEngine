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
        lineMesh.PixelShader = "solidRed_pixel";

        lineMesh.Position = mix(s, e, 0.5);
        lineMesh.Rotation = MathHelper::FindLookAtRotation(s, e);

        lineMesh.Size.z = distance(s, e);

        lineMesh.Size.x = th;
        lineMesh.Size.y = th;

    }

    void Draw() override
    {
        lineMesh.FinalizeFrameData();
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

    // This method is called to produce a snapshot for the render thread. It:
    // 1. Removes expired commands from 'commands'
    // 2. Copies pointers to the remaining commands into 'finalizedCommands'
    static void Finalize()
    {
        std::lock_guard<std::mutex> lock(mainLock);

        // Clear the previous snapshot.
        finalizedCommands.clear();

        // Iterate over commands and build a new container for active commands.
        // Use index-based iteration so we can erase elements while iterating.
        for (auto it = commands.begin(); it != commands.end(); )
        {
            // If DrawTime.Wait() is true, then the command expired.
            if ((*it)->DrawTime.Wait() == false)
            {
                // Erase expired command from the owning container.
                it = commands.erase(it);
            }
            else
            {
                // The command is active: add its raw pointer to the snapshot.
                finalizedCommands.push_back(it->get());
                ++it;
            }
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