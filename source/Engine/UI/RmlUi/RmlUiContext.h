#pragma once

#include <RmlUi/Core.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <SDL2/SDL.h>

class SystemInterface_SDL;
class RenderInterface_GL3;

class RmlUiContext 
{
public:


	static inline RmlUiContext* Main = nullptr;


    // Callbacks for game update (only if no modal blocks input) and custom render (optional)
    using GameUpdateCallback = std::function<void(float delta)>;
    using PreRenderCallback = std::function<void()>;  // Called before RmlUi Render()

    explicit RmlUiContext(SDL_Window* sdl_window, int initial_width, int initial_height,
        bool enable_debugger = true);
    ~RmlUiContext();

    // Must call once after construction
    bool Initialize();

    // Call every frame
    void Update(float delta_seconds);
    void Render();  // Renders UI on top of your scene

	void PushModal(Rml::ElementDocument* doc);
	void PopModal();
	void RemoveFromModalFromStack(Rml::ElementDocument* doc);
	void ClearModals();

    // Event forwarding: call this for every SDL_Event your game processes
    void ProcessEvent(SDL_Event& event);

    // Resize handling
    void OnResize(int new_width, int new_height);

    // Document management
    Rml::ElementDocument* LoadDocument(const std::string& filename);
    void ShowDocument(Rml::ElementDocument* doc, bool modal = false, bool pull_to_front = true);
    void HideDocument(Rml::ElementDocument* doc);
    void UnloadDocument(Rml::ElementDocument* doc);  // Hide + Close
	void UnloadAllDocuments();

    void PerformModalBackAction();

    void ReloadAllSttyles();

    Rml::Element* GetElementById(Rml::ElementDocument* doc, const std::string& id);
    void AddEventListener(Rml::ElementDocument* doc, const std::string& event_name, Rml::EventListener* listener);

    // Utilities
    bool IsUIBlockingInput() const;  // Any modal open? → pause game logic
    Rml::Context* GetRmlContext() const { return context_; }

    void SetGameUpdateCallback(GameUpdateCallback cb) { game_update_cb_ = std::move(cb); }
    void SetPreRenderCallback(PreRenderCallback cb) { pre_render_cb_ = std::move(cb); }

private:
    SDL_Window* window_;
    int width_, height_;
    bool enable_debugger_;


    Rml::Context* context_ = nullptr;

    std::unordered_map<std::string, Rml::ElementDocument*> loaded_docs_;

    GameUpdateCallback game_update_cb_;
    PreRenderCallback  pre_render_cb_;

	std::stack<Rml::ElementDocument*> modal_stack_;

};