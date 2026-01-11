#include "UiLocationTileDrop.h"
#include <Time.hpp>
#include <SoundPlayer.h>
#include <EngineMain.h>

UiLocationTileDrop::UiLocationTileDrop(std::string title)
{

	text = make_shared<UiText>();
    text->fontSize = 120;
    text->origin = vec2(0.5f);
    text->pivot = vec2(0.5f);
    text->text = title;
    AddChild(text);
    
    origin = vec2(0.5f);
    pivot = vec2(0.5f);

    position = vec2(0, -200);

    color = vec4(0);

}

void UiLocationTileDrop::Update()
{

    if (EngineMain::MainInstance->Paused) return;

    time += Time::DeltaTimeF;
    float blend = GetBlendValue(time);

    color = vec4(1,1,1,blend);

    UiCanvas::Update();

    if (time > duration)
    {
        parent->RemoveChild(shared_from_this());
    }

}

void UiLocationTileDrop::PlayTitleCard(std::string title)
{

    SoundPlayer::PlayOneshot("event:/General/LocationTitleDrop", 9, 1, false);

    EngineMain::MainInstance->Viewport.AddChild(make_shared<UiLocationTileDrop>(title));

}

float UiLocationTileDrop::GetBlendValue(float time) const
{
    if (time <= 0.0f)
        return 0.0f;

    if (time >= duration)
        return 0.0f;

    // Fade in
    if (time < blendIn)
    {
        return time / blendIn;
    }

    // Fade out
    if (time > duration - blendOut)
    {
        return (duration - time) / blendOut;
    }

    // Fully visible
    return 1.0f;
}
