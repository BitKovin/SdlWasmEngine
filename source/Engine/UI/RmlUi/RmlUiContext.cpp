#include "RmlUiContext.h"
#include "../../gl.h"
#include <RmlUi/Debugger.h>
#include "Backends/RmlUi_Backend.h"
#include "Backends/RmlUi_Renderer_GL3.h"
#include "Backends/RmlUi_Platform_SDL.h"

RmlUiContext::RmlUiContext(SDL_Window* sdl_window, int initial_width, int initial_height, bool enable_debugger)
    : window_(sdl_window), width_(initial_width), height_(initial_height), enable_debugger_(enable_debugger)
{
    // Assume GL is already loaded and current
}

RmlUiContext::~RmlUiContext() {
    for (auto& pair : loaded_docs_) {
        if (pair.second) pair.second->Close();
    }
    loaded_docs_.clear();

    if (context_) Rml::RemoveContext(context_->GetName());

    Rml::Shutdown();

    delete render_interface_;
    delete system_interface_;
}

bool RmlUiContext::Initialize() 
{

	Backend::Initialize("RmlUi SDL2 OpenGL Context", width_, height_, true);

    system_interface_ = (SystemInterface_SDL*)Backend::GetSystemInterface();
    Rml::SetSystemInterface(system_interface_);

    render_interface_ = (RenderInterface_GL3*)Backend::GetRenderInterface();
    Rml::SetRenderInterface(render_interface_);
    

    if (!Rml::Initialise()) 
        return false;

    context_ = Rml::CreateContext("main", Rml::Vector2i(width_, height_));
    if (!context_) 
        return false;

    if (enable_debugger_) {
        Rml::Debugger::Initialise(context_);
    }


    // Load common fonts (add more as needed)
    Rml::LoadFontFace("GameData/fonts/Kingthings_Calligraphica_2.ttf");

    return true;
}

void RmlUiContext::ProcessEvent(SDL_Event& event) {
	RmlSDL::InputEventHandler(context_, window_, event);
}

void RmlUiContext::Update(float delta_seconds) {
    context_->Update();

    if (game_update_cb_ && !IsUIBlockingInput()) {
        game_update_cb_(delta_seconds);
    }
}

void RmlUiContext::Render() {
    if (pre_render_cb_) pre_render_cb_();  // Optional: e.g., dim screen on pause
	render_interface_->BeginFrame();
    context_->Render();
	render_interface_->EndFrame();
}

void RmlUiContext::OnResize(int new_width, int new_height) 
{


    float ui_scale_factor = static_cast<float>(new_height) / 1080.0f;
    context_->SetDensityIndependentPixelRatio(ui_scale_factor);

    width_ = new_width;
    height_ = new_height;
    glViewport(0, 0, width_, height_);
    if (context_) context_->SetDimensions(Rml::Vector2i(width_, height_));
    if (render_interface_) render_interface_->SetViewport(width_, height_);
}

Rml::ElementDocument* RmlUiContext::LoadDocument(const std::string& filename) {
    auto doc = context_->LoadDocument(filename.c_str());
    if (doc) {
        loaded_docs_[filename] = doc;
    }
    return doc;
}

void RmlUiContext::ShowDocument(Rml::ElementDocument* doc, bool modal, bool pull_to_front) {
    if (doc) {
        doc->Show(modal ? Rml::ModalFlag::Modal : Rml::ModalFlag::None);
        if (pull_to_front) doc->PullToFront();
        context_->ProcessMouseLeave();  // Prevent stuck hover
    }
}

void RmlUiContext::HideDocument(Rml::ElementDocument* doc) {
    if (doc) doc->Hide();
}

void RmlUiContext::UnloadDocument(Rml::ElementDocument* doc) {
    if (!doc) return;
    doc->Close();
    for (auto it = loaded_docs_.begin(); it != loaded_docs_.end(); ++it) {
        if (it->second == doc) {
            loaded_docs_.erase(it);
            break;
        }
    }
}

Rml::Element* RmlUiContext::GetElementById(Rml::ElementDocument* doc, const std::string& id) {
    return doc ? doc->GetElementById(id.c_str()) : nullptr;
}

void RmlUiContext::AddEventListener(Rml::ElementDocument* doc, const std::string& event_name, Rml::EventListener* listener) {
    if (doc && listener) {
        doc->AddEventListener(event_name.c_str(), listener, false);
    }
}

bool RmlUiContext::IsUIBlockingInput() const {
    return context_ && context_->GetFocusElement();
    // Alternative: track manually if needed
}