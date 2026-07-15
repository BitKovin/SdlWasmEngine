#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>

#include <PauseGameManager.hpp>

#include "UiVideoSettings.h"
#include "UiInputSettings.h"
#include "UiGameSettings.h"
#include "UiSoundSettings.h"

class UiSettingsMenu : public UiCanvas
{
public:

	std::shared_ptr<UiElement> parentMenu;

	UiSettingsMenu(std::shared_ptr<UiElement> parentMenu_)
	{

		FocusTrap = true;

		this->parentMenu = parentMenu_;

		background = make_shared<UiImage>();
		background->color = vec4(0.5f, 0.0f, 0.0f, 0.5f);
		AddChild(background);

		optionsBox = std::make_shared<UiVerticalBox>();
		optionsBox->origin = vec2(0.5f);
		optionsBox->pivot = vec2(0.5f);

		gameButton = MakeButton("Gameplay Settings");
		inputButton = MakeButton("Input Settings");
		videoButton = MakeButton("Video Settings");
		soundButton = MakeButton("Sound Settings");
		backButton = MakeButton("Back");

		optionsBox->AddChild(gameButton);
		optionsBox->AddChild(inputButton);
		optionsBox->AddChild(videoButton);
		optionsBox->AddChild(soundButton);
		optionsBox->AddChild(backButton);

		AddChild(optionsBox);

		gameButton->onClick = [&]()
			{
				visible = false;

				EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiGameSettings>(shared_from_this()));

			};

		videoButton->onClick = [&]()
			{
				visible = false;

				EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiVideoSettings>(shared_from_this()));

			};

		inputButton->onClick = [&]()
			{
				visible = false;

				EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiInputSettings>(shared_from_this()));

			};

		soundButton->onClick = [&]()
			{
				visible = false;

				EngineMain::MainInstance->Viewport.AddChild(std::make_shared<UiSoundSettings>(shared_from_this()));

			};

		backButton->onClick = [&]()
			{
				parentMenu->visible = true;
				GameSettings::Instance().Video.FromCurrentState();
				GameSettings::Instance().ApplyAll();
				GameSettings::Instance().SaveToFile();
				RemoveFromParent();
			};

		UpdateChildrenOffsetRecursive();

	}

	void OnNavCancel() override
	{
		backButton->onClick();
	}

	void FinalizeChildren() override
	{

		background->size = GetSize();

		UiCanvas::FinalizeChildren();
	}

private:

	std::shared_ptr<UiVerticalBox> optionsBox;

	std::shared_ptr<UiButton> gameButton;
	std::shared_ptr<UiButton> inputButton;
	std::shared_ptr<UiButton> videoButton;
	std::shared_ptr<UiButton> soundButton;
	std::shared_ptr<UiButton> backButton;

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

