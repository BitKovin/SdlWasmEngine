#include "RmlUiContext.h"
#include "../../gl.h"
#include <RmlUi/Debugger.h>
#include "Backends/RmlUi_Backend.h"
#include "Backends/RmlUi_Renderer_GL3.h"
#include "Backends/RmlUi_Platform_SDL.h"
#include "RmlUiEvents.h"
#include "../../Input.h"

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

void RmlUiContext::Update(float delta_seconds) 
{

    if (Input::GetAction("ui_confirm")->Pressed())
    {
        context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_RETURN, RmlSDL::GetKeyModifierState());
    }
    if (Input::GetAction("ui_cancel")->Pressed())
    {
        //context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_ESCAPE, RmlSDL::GetKeyModifierState());
		PerformModalBackAction();
	}
    if(Input::GetAction("ui_up")->Pressed())
    {
        context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_UP, RmlSDL::GetKeyModifierState());
	}
    if (Input::GetAction("ui_down")->Pressed())
    {
        context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_DOWN, RmlSDL::GetKeyModifierState());
    }
    if (Input::GetAction("ui_left")->Pressed())
    {
        context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_LEFT, RmlSDL::GetKeyModifierState());
    }
    if (Input::GetAction("ui_right")->Pressed())
    {
        context_->ProcessKeyDown(Rml::Input::KeyIdentifier::KI_RIGHT, RmlSDL::GetKeyModifierState());
	}



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

void RmlUiContext::PushModal(Rml::ElementDocument* doc)
{
    if (doc)
    {
        modal_stack_.push(doc);
		ShowDocument(doc, true, true);
	}
}

void RmlUiContext::PopModal()
{
    if (!modal_stack_.empty())
    {
		modal_stack_.top()->Hide();
        modal_stack_.pop();

        auto top = modal_stack_.top();

        if (top)
        {
            top->Focus();
        }


	}
}

void RmlUiContext::RemoveFromModalFromStack(Rml::ElementDocument* doc)
{
    if (!doc)
        return;
    std::stack<Rml::ElementDocument*> temp_stack;
    // Pop elements until we find the target document
    while (!modal_stack_.empty())
    {
        Rml::ElementDocument* top_doc = modal_stack_.top();
        modal_stack_.pop();
        if (top_doc == doc)
        {
            top_doc->Hide();  // Hide the document being removed
            break;  // Exit the loop once we've found and removed the target document
        }
        else
        {
            temp_stack.push(top_doc);  // Store other documents temporarily
        }
    }
    // Push back the other documents onto the original stack
    while (!temp_stack.empty())
    {
        modal_stack_.push(temp_stack.top());
        temp_stack.pop();
	}
}

void RmlUiContext::ClearModals()
{
    while (!modal_stack_.empty())
    {
        modal_stack_.top()->Hide();
        modal_stack_.pop();
	}
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
    auto* doc = context_->LoadDocument(filename.c_str());
    if (doc) {
        loaded_docs_[filename] = doc;

        using namespace RmlUiEvents;


        doc->AddEventListener("touchstart",
            new LambdaListener([](Rml::Element* e) {
                if (!e)
                    return;

                auto focusedElement = e->GetOwnerDocument()->GetFocusLeafNode();

                if (focusedElement)
                    focusedElement->Blur();
                })
        );

        doc->AddEventListener("mouseover",
            new LambdaListener([](Rml::Element* e) {
                if (!e)
                    return;

                const Rml::Property* focus_prop = e->GetProperty("focus"); 
                    if (!focus_prop) return; // Read keyword value as int 

                int focus_value = focus_prop->Get<int>(); // Compare with Style::Focus values 
                if (focus_value != (int)Rml::Style::Focus::None) 
                {

                    auto focusedElement = e->GetOwnerDocument()->GetFocusLeafNode();


                    if (focusedElement)
                        focusedElement->Blur();
                }
                })
        );


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
        if (it->second == doc) 
        {
            loaded_docs_.erase(it);
            break;
        }
    }
}

void RmlUiContext::UnloadAllDocuments()
{

    ClearModals();

	auto current_docs = loaded_docs_;  // Copy to avoid modification during iteration

    for (auto doc : current_docs)
    {
        UnloadDocument(doc.second);
    }

	loaded_docs_.clear();

}

void RmlUiContext::PerformModalBackAction()
{


	if (modal_stack_.empty())
        return;

	auto top = modal_stack_.top();

    Rml::Element* button = top->GetElementById("backBtn");

    if (button)
    {
        // Dispatch a "click" event
        button->DispatchEvent("click", {});
    }
    else
    {
		//PopModal();
    }



}

void RmlUiContext::ReloadAllSttyles()
{

    for (auto doc : loaded_docs_)
    {
        doc.second->ReloadStyleSheet();
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