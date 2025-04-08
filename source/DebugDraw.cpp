#include "DebugDraw.hpp"

// Definitions for static members, usually in DebugDraw.cpp:
std::mutex DebugDraw::mainLock;
std::vector<std::unique_ptr<DebugDrawCommand>> DebugDraw::commands;
std::vector<DebugDrawCommand*> DebugDraw::finalizedCommands;