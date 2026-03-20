#pragma once

#include <UI/UiProgressBar.hpp>
#include <UI/UiText.hpp>
#include <UI/UiContentBox.hpp>

#include <Systems/ScoreSystem/ScoreSystem.h>

#include <string>

class UiScoreIndicator : public UiContentBox
{
public:
	UiScoreIndicator()
	{
		progressBar = std::make_shared<UiProgressBar>();
		text = std::make_shared<UiText>();

		progressBar->pivot = vec2(0.5f);
		progressBar->origin = vec2(0.5f);
		progressBar->rotation = -90;
		progressBar->size = vec2(100);
		text->pivot = vec2(0.5f);
		text->origin = vec2(0.5f);
		text->fontSize = 102;

		AddChild(progressBar);
		AddChild(text);

		pivot = vec2(0.5f);
		origin = vec2(0.5f, 0.0f);
		position = vec2(0, 50);

	}

	void Update() override
	{

		text->text = ScoreSystem::Instance().tierName();
		progressBar->Progress = ScoreSystem::Instance().tierProgress();
		progressBar->color = vec4(1,0,0,1.0f);

		UiContentBox::Update();

	}

private:

	std::shared_ptr<UiProgressBar> progressBar;
	std::shared_ptr<UiText> text;

};

