#pragma once

#include <UI/UiVerticalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiProgressBar.hpp>

// ---------------------------------------------------------------------------
// HudStatElement
//
// One column of the retro HUD status bar: a small caps label ("STAMINA")
// stacked above a bigger value ("3.0"), both horizontally centered. The
// column has a FIXED size (set at construction) regardless of how wide the
// label/value text is -- this is what keeps STAMINA / HEALTH / AMMO evenly
// spaced no matter how many digits the value has ("3.0" vs "100" vs "27").
//
// To add a new stat to the HUD: make another HudStatElement, AddChild it to
// the HudStatusBar, and call SetValue(...) on it from PlayerHud::Update().
// No layout code needs to change.
// ---------------------------------------------------------------------------
class HudStatElement : public UiElement
{
public:
    std::shared_ptr<UiText> Label;
    std::shared_ptr<UiText> Value; 
	std::shared_ptr<UiProgressBar> ProgressBar;

    // fixedSize    -- the column's locked width/height (see GetSize() below).
    // accentColor  -- tint for the value text only (label stays neutral),
    //                 e.g. green stamina / red health / yellow ammo.
    HudStatElement(const std::string& labelText, glm::vec2 fixedSize, glm::vec4 accentColor = glm::vec4(1.f))
    {
        size = fixedSize;

        Label = make_shared<UiText>();
        Label->text = labelText;
        Label->fontSize = 52;
        Label->origin = vec2(0.5f, 0.f);
        Label->pivot = vec2(0.5f, 0.f);
        Label->color = vec4(0.8f, 0.8f, 0.8f, 1.f);
        Label->outlineEnabled = true;
        Label->outlineWidth = 2.f;
        Label->outlineColor = vec4(0.f, 0.f, 0.f, 1.f);
		Label->shadowEnabled = true;
        Label->shadowOffset = vec2(2.f, 2.f);
        Label->shadowSoftness = 5.f;
		Label->shadowSpread = 4.f;
		Label->color = mix(Label->color, accentColor, 0.4f);
        AddChild(Label);

        Value = make_shared<UiText>();
        Value->text = "";
        Value->fontSize = 65;
        Value->origin = vec2(0.5f, 1.f);
        Value->pivot = vec2(0.5f, 1.f);
        Value->color = accentColor;
        Value->outlineEnabled = true;
        Value->outlineWidth = 2.0f;
        Value->outlineColor = vec4(0.f, 0.f, 0.f, 1.f);
		Value->shadowEnabled = true;
        Value->shadowOffset = vec2(2.f, 2.f);
        Value->shadowSoftness = 5.f;
        Value->shadowSpread = 4.f;
        AddChild(Value);

        float progressBarWidth = Label->GetSize().x + 4;

        progressBarWidth = std::clamp(progressBarWidth, 120.0f, 240.0f);

        ProgressBar = make_shared<UiProgressBar>();
		ProgressBar->position = vec2(0.f, Label->GetSize().y - 2);
        ProgressBar->origin = vec2(0.5f, 0.0f);
        ProgressBar->Progress = 0.5f;
        ProgressBar->pivot = vec2(0.5f, 0.5f);
        ProgressBar->size = vec2(progressBarWidth + 6, 5);
		ProgressBar->color = accentColor * vec4(0.5f, 0.5f, 0.5f, 1.f);
        ProgressBar->BackgroundColor = ProgressBar->color * vec4(0.4f, 0.4f, 0.4f, 1.f);
		ProgressBar->ProgressImage = "GameData/textures/generic/white.png";
		ProgressBar->BackgroundImage = "GameData/textures/generic/white.png";
		ProgressBar->outlineEnabled = true;
		ProgressBar->shadowEnabled = true;
		ProgressBar->shadowOffset = vec2(1.f, 1.f);


        AddChild(ProgressBar);

    }

    virtual ~HudStatElement() = default;

    // Locks this column to `size` no matter what UiContentBox would
    // otherwise compute from the children -- every stat column must stay
    // the same width so they don't drift apart as values change length.
    glm::vec2 GetSize() override { return size; }

    void SetValue(const std::string& value) { Value->text = value; }
    void SetLabel(const std::string& label) { Label->text = label; }
};
