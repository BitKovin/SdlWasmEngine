#pragma once

#include <RmlUi/Core.h>
#include <functional>
#include <unordered_map>
#include <string>

namespace RmlUiEvents {

    // Generic lambda listener
    struct LambdaListener : Rml::EventListener {
        std::function<void(Rml::Element*)> func;
        LambdaListener(std::function<void(Rml::Element*)> f) : func(f) {}
        void ProcessEvent(Rml::Event& event) override {
            func(event.GetTargetElement());
        }
    };

    // New lambda listener that passes the event
    struct LambdaListener2 : Rml::EventListener {
        std::function<void(Rml::Event&)> func;
        LambdaListener2(std::function<void(Rml::Event&)> f) : func(f) {}
        void ProcessEvent(Rml::Event& event) override {
            func(event);
        }
    };

    inline void onEvent(Rml::ElementDocument* doc, const std::string& element_id,
        const std::string& event, std::function<void(Rml::Event&)> callback)
    {
        Rml::Element* elem = doc->GetElementById(element_id);
        if (elem)
            elem->AddEventListener(event.c_str(), new LambdaListener2(callback));
    }

    // Base function to attach any event by element ID
    inline void onEvent(Rml::ElementDocument* doc, const std::string& element_id,
                        const std::string& event, std::function<void(Rml::Element*)> callback)
    {
        Rml::Element* elem = doc->GetElementById(element_id);
        if (elem)
            elem->AddEventListener(event.c_str(), new LambdaListener(callback));
    }

    // Helper for button click
    inline void onClick(Rml::ElementDocument* doc, const std::string& element_id,
                        std::function<void()> callback)
    {
        onEvent(doc, element_id, "click", [callback](Rml::Element*){ callback(); });
    }

    // Helper for input change (passes value as string)
    inline void onChange(Rml::ElementDocument* doc, const std::string& element_id,
        std::function<void(const std::string&)> callback)
    {
        onEvent(doc, element_id, "change", [callback](Rml::Element* e) {
            auto* control = dynamic_cast<Rml::ElementFormControl*>(e);
            if (control) callback(control->GetValue());
            });
    }
}
