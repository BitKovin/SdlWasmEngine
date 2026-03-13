/*
 * RmlUiContext.h — updated for bgfx backend
 *
 * The only change from the GL3 version is that the render interface
 * pointer type is RenderInterface_BGFX* instead of RenderInterface_GL3*.
 */

#pragma once

#include <RmlUi/Core.h>
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <stack>
#include <functional>

class RenderInterface_BGFX;
class SystemInterface_SDL;

class RmlUiContext
{
public:

	static inline RmlUiContext* Main = nullptr; // global pointer for convenience; can be set to multiple contexts if needed

    RmlUiContext(SDL_Window* sdl_window, int initial_width, int initial_height, bool enable_debugger = false);
    ~RmlUiContext();

    bool Initialize();

    void ProcessEvent(SDL_Event& event);
    void Update(float delta_seconds);
    void Render();

    // Modal stack
    void PushModal(Rml::ElementDocument* doc);
    void PopModal();
    void RemoveFromModalFromStack(Rml::ElementDocument* doc);
    void ClearModals();

    // Window resize
    void OnResize(int new_width, int new_height);

    // Document management
    Rml::ElementDocument* LoadDocument(const std::string& filename);
    void ShowDocument(Rml::ElementDocument* doc, bool modal = false, bool pull_to_front = false);
    void HideDocument(Rml::ElementDocument* doc);
    void UnloadDocument(Rml::ElementDocument* doc);
    void UnloadAllDocuments();

    void PerformModalBackAction();
    void ReloadAllSttyles();  // [sic] — preserves original typo from user code

    // Element helpers
    Rml::Element* GetElementById(Rml::ElementDocument* doc, const std::string& id);
    void AddEventListener(Rml::ElementDocument* doc, const std::string& event_name,
                          Rml::EventListener* listener);

    bool IsUIBlockingInput() const;

    Rml::Context* GetContext() const { return context_; }

    // Callbacks
    void SetGameUpdateCallback(std::function<void(float)> cb) { game_update_cb_ = cb; }
    void SetPreRenderCallback(std::function<void()> cb)       { pre_render_cb_ = cb; }

private:
    SDL_Window*  window_ = nullptr;
    int          width_  = 0;
    int          height_ = 0;
    bool         enable_debugger_ = false;

    Rml::Context*           context_          = nullptr;
    RenderInterface_BGFX*   render_interface_  = nullptr;
    SystemInterface_SDL*    system_interface_  = nullptr;

    std::unordered_map<std::string, Rml::ElementDocument*> loaded_docs_;
    std::stack<Rml::ElementDocument*> modal_stack_;

    std::function<void(float)> game_update_cb_;
    std::function<void()>      pre_render_cb_;
};
