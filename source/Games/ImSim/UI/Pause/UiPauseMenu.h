#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>

#include <PauseGameManager.hpp>

#include "UiSettingsMenu.h"

class UiPauseMenu : public UiCanvas
{
public:
	UiPauseMenu()
	{

		FocusTrap = true;

		background = make_shared<UiImage>();
		background->color = vec4(0.5f, 0.0f, 0.0f, 0.5f);
		AddChild(background);

		optionsBox = std::make_shared<UiVerticalBox>();
		optionsBox->origin = vec2(0.5f);
		optionsBox->pivot = vec2(0.5f);
		
		resumeButton = MakeButton("Resume");
		saveButton = MakeButton("Save Game");
		loadButton = MakeButton("Load Game");
		settingsButton = MakeButton("Settings");
		menuButton = MakeButton("Main Menu");

		optionsBox->AddChild(resumeButton);
		optionsBox->AddChild(saveButton);
		optionsBox->AddChild(loadButton);
		optionsBox->AddChild(settingsButton);
		optionsBox->AddChild(menuButton);

		optionsBox->rotation = 30;

		AddChild(optionsBox);

		resumeButton->onClick = [&]() 
			{
				PauseGameManager::SetGamePaused(false);
				Input::GetAction("pause")->CleanPressed();
			};

		settingsButton->onClick = [&]()
			{
				
				EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiSettingsMenu>(shared_from_this()));
				visible = false;
			};

	}
	
	void OnNavCancel() override
	{
		resumeButton->onClick();
	}

	void FinalizeChildren() override
	{

		background->size = GetSize();

		UiCanvas::FinalizeChildren();
	}

private:

	std::shared_ptr<UiVerticalBox> optionsBox;

	std::shared_ptr<UiButton> resumeButton;
	std::shared_ptr<UiButton> saveButton;
	std::shared_ptr<UiButton> loadButton;
	std::shared_ptr<UiButton> settingsButton;
	std::shared_ptr<UiButton> menuButton;

	std::shared_ptr<UiImage> background;

	std::shared_ptr<UiButton> MakeButton(std::string text)
	{

		std::shared_ptr<UiButton> btn = make_shared<UiButton>();
		std::shared_ptr<UiText> txt = make_shared<UiText>();

		btn->size = vec2(400, 70);
		
		txt->text = text;

		txt->pivot = vec2(0.5f);
		txt->origin = vec2(0.5f);

		btn->AddChild(txt);

		return btn;
	}

};

