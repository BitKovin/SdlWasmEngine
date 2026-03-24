#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_bgfx.h"
#include "Input.h"
#include "gl.h"

inline void ImStartFrame()
{
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (Input::LockCursor)
    {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
    }
    else
    {
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDL2_NewFrame();      // ← stays the same
    ImGui_Implbgfx_NewFrame();      // ← was ImGui_ImplOpenGL3_NewFrame()

    ImGui::NewFrame();

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::DockSpaceOverViewport();
    ImGui::PopStyleColor(2);
}

inline void RenderImGui()
{
    // Rendering
    ImGui::Render();

    ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
}