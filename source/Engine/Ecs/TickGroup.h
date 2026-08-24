#pragma once
// TickGroup.h
//
// The four fixed points in EngineMain::GameUpdate() where ECS systems run.
// A plain enum, not a dynamic registry - these are specific known hooks into
// an existing loop, not arbitrary user-defined stages.

#include <cstdint>
#include <string_view>

enum class TickGroup : uint8_t
{
    PreUpdate,
    PostUpdate,
    PostAsyncUpdate,
    PostLateUpdate,
    Count
};

inline constexpr size_t TickGroupCount = (size_t)TickGroup::Count;

inline constexpr std::string_view TickGroupName(TickGroup group)
{
    switch (group)
    {
        case TickGroup::PreUpdate:       return "PreUpdate";
        case TickGroup::PostUpdate:      return "PostUpdate";
        case TickGroup::PostAsyncUpdate: return "PostAsyncUpdate";
        case TickGroup::PostLateUpdate:  return "PostLateUpdate";
        default:                         return "Unknown";
    }
}
