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

		resolutions = std::make_shared<UiDropdown>();
		resolutions->SetOptions(model.resolutions);
		resolutions->size = vec2(400,70);
		resolutionsHolder = GetSettingRow("resolution", resolutions);


		optionsBox->AddChild(resolutionsHolder);
		optionsBox->AddChild(backButton);

		AddChild(optionsBox);

		backButton->onClick = [&]()
			{
				parentMenu->visible = true;
				RemoveFromParent();
			};

		UpdateChildrenOffsetRecursive();

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

};

