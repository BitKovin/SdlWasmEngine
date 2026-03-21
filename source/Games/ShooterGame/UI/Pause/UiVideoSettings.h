#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiDropdown.hpp>

#include <PauseGameManager.hpp>

#include "Settings/VideoSettings.h"

class UiVideoSettings : public UiCanvas
{
public:

	std::shared_ptr<UiElement> parentMenu;

	UiVideoSettings(std::shared_ptr<UiElement> parentMenu_)
	{

		FocusTrap = true;

		this->parentMenu = parentMenu_;

		background = make_shared<UiImage>();
		background->color = vec4(0.5f, 0.0f, 0.0f, 0.5f);
		AddChild(background);

		optionsBox = std::make_shared<UiVerticalBox>();
		optionsBox->origin = vec2(0.5f);
		optionsBox->pivot = vec2(0.5f);


		backButton = MakeButton("Back");

		VideoSettingsModel model;

		VideoSettings::InitModelData(model);

		if (std::find(model.resolutions.begin(), model.resolutions.end(), model.selected_resolution) == model.resolutions.end())
		{
			model.resolutions.push_back(model.selected_resolution);
		}

		int selectedIndex = std::distance(model.resolutions.begin(), std::find(model.resolutions.begin(), model.resolutions.end(), model.selected_resolution));

		resolutions = std::make_shared<UiDropdown>();
		resolutions->SetOptions(model.resolutions);
		resolutions->SetSelectedIndex(selectedIndex);
		resolutions->size = vec2(400,70);
		resolutions->onSelectionChanged = UiVideoSettings::UpdateResolution;
		resolutionsHolder = GetSettingRow("Resolution", resolutions);

		windowMode = std::make_shared<UiDropdown>();
		windowMode->SetOptions({"windowed","fullscreen", "borderless"});

		int windowOption = 0;
		Uint32 flags = SDL_GetWindowFlags(EngineMain::MainInstance->Window);
		if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
			windowOption = 2;
		else if (flags & SDL_WINDOW_FULLSCREEN)
			windowOption = 1;

		windowMode->SetSelectedIndex(windowOption);
		windowMode->size = vec2(400, 70);
		windowMode->onSelectionChanged = UiVideoSettings::UpdateWindowMode;
		windowModeHolder = GetSettingRow("Window Mode", windowMode);


		optionsBox->AddChild(resolutionsHolder);
		optionsBox->AddChild(windowModeHolder);
		optionsBox->AddChild(backButton);

		AddChild(optionsBox);

		backButton->onClick = [&]()
			{
				parentMenu->visible = true;
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

	std::shared_ptr<UiDropdown> resolutions;
	std::shared_ptr<UiHorizontalBox> resolutionsHolder;

	std::shared_ptr<UiDropdown> windowMode;
	std::shared_ptr<UiHorizontalBox> windowModeHolder;

	std::shared_ptr<UiDropdown> aaMode;
	std::shared_ptr<UiHorizontalBox> aaModeHolder;

	std::shared_ptr<UiButton> backButton;

	std::shared_ptr<UiImage> background;

	std::shared_ptr<UiHorizontalBox> GetSettingRow(std::string name, std::shared_ptr<UiElement> element)
	{

		std::shared_ptr<UiText> txt = std::make_shared<UiText>();
		txt->text = name;

		txt->pivot = vec2(0);
		txt->origin = vec2(0);

		element->pivot = vec2(0);
		element->origin = vec2(0);
		
		std::shared_ptr<UiHorizontalBox> horBox = std::make_shared<UiHorizontalBox>();
		horBox->AddChild(txt);
		horBox->AddChild(element);

		return horBox;
	}

	std::shared_ptr<UiButton> MakeButton(std::string text)
	{

		std::shared_ptr<UiButton> btn = std::make_shared<UiButton>();
		std::shared_ptr<UiText> txt = std::make_shared<UiText>();

		btn->size = vec2(400, 70);

		txt->text = text;

		txt->pivot = vec2(0.5f);
		txt->origin = vec2(0.5f);

		btn->AddChild(txt);

		return btn;
	}

	static inline void UpdateResolution(int index, const std::string& value)
	{

		// Parse resolution string, expected format: "WIDTHxHEIGHT" (e.g., "1920x1080")
		size_t xPos = value.find('x');
		if (xPos == std::string::npos) return;

		int width = std::stoi(value.substr(0, xPos));
		int height = std::stoi(value.substr(xPos + 1));

		// Assuming gWindow is your SDL_Window* accessible globally
		SDL_Window* gWindow = EngineMain::MainInstance->Window;

		// For fullscreen modes, we need to change the display mode
		Uint32 flags = SDL_GetWindowFlags(gWindow);
		if (flags & SDL_WINDOW_FULLSCREEN) {
			// In fullscreen, we must change the display mode
			SDL_DisplayMode mode;
			mode.format = SDL_PIXELFORMAT_UNKNOWN; // let SDL choose
			mode.w = width;
			mode.h = height;
			mode.refresh_rate = 0; // use current refresh rate
			mode.driverdata = nullptr;

			SDL_SetWindowDisplayMode(gWindow, &mode);
			// Toggle fullscreen off and on to apply? Usually SetDisplayMode works immediately
			// But we can also just set the mode and it should apply
		}
		else {
			// Windowed mode: just set the size
			SDL_SetWindowSize(gWindow, width, height);
		}
	}

	static inline void UpdateWindowMode(int index, const std::string& value)
	{

		if (index == 0)
		{
			SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, 0);
		}
		else if(index == 2)
		{
			SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else
		{
			SDL_SetWindowFullscreen(EngineMain::MainInstance->Window, SDL_WINDOW_FULLSCREEN);
		}


	}

};

