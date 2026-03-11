#pragma once
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiCustomShaderImage.hpp>
#include <UI/UiText.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiImage.hpp>

#include "../../Entities/Enemy/IEnemy.h"
#include <Entity.h>

class UiDebuffIcon : public UiCustomShaderImage
{

public:

	UiDebuffIcon(Debuff& debuff)
	{
		size = glm::vec2(128.f, 128.f);

		Textures["u_Texture"] = AssetRegistry::GetTextureFromFile(debuff.iconPath)->getHandle();
		Scalars["u_progress"] = debuff.GetProgress();
		Scalars["u_timeRel"] = debuff.GetTimeRel();
		color = glm::vec4(debuff.uiColor, 1.0f);
		PixelShader = "game/debuff";

		if(debuff.uiShowStacks)
		{
			auto stackText = std::make_shared<UiText>();
			stackText->position = glm::vec2(0.0f, 0.0f);
			stackText->fontSize = 40;
			stackText->pivot = glm::vec2(0.5f);
			stackText->origin = glm::vec2(0.85f);
			stackText->text = std::to_string(debuff.GetStacks());

			stackText->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

			AddChild(stackText);
		
		}

	}
	

};

class UiNpcDebuffs : public UiHorizontalBox
{

public:

	IEnemy* Target = nullptr;

	UiNpcDebuffs()
	{

	}
	void Update() override
	{

		ClearChildren();
		
		auto debuffs = Target->GetDebuffsList();

		if (debuffs.empty())
		{

			auto icon = std::make_shared<UiImage>();
			icon->size = glm::vec2(0.f, 128.f);
			icon->color = glm::vec4(0.f, 0.f, 0.f, 0.f);
			AddChild(icon);

			UiHorizontalBox::Update();
			return;
		}

		for (auto& d : debuffs)
		{
			if (d->iconPath.empty()) continue;

			auto icon = std::make_shared<UiDebuffIcon>(*d);
			AddChild(icon);

		}
		UiHorizontalBox::Update();
	}

};

class UiNpcStatus : public UiVerticalBox
{
public:
	IEnemy* Target = nullptr;
	Entity* Owner = nullptr;

	std::shared_ptr<UiImage> healthBar = nullptr;
	std::shared_ptr<UiImage> healthBarBg = nullptr;

	UiNpcStatus(IEnemy* target)
	{

		auto debuffs = std::make_shared<UiNpcDebuffs>();
		debuffs->Target = target;
		debuffs->pivot.x = 0.5f;
		debuffs->origin.x = 0.5f;
		AddChild(debuffs);

		Target = target;
		healthBarBg = std::make_shared<UiImage>();
		healthBarBg->ImagePath = "GameData/textures/ui/white.png";
		healthBarBg->size = glm::vec2(600, 40);
		healthBarBg->pivot.x = 0.5f;
		healthBarBg->origin.x = 0.5f;
		healthBarBg->color = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
		AddChild(healthBarBg);

		healthBar = std::make_shared<UiImage>();
		healthBar->ImagePath = "GameData/textures/ui/white.png";
		healthBar->size = glm::vec2(600, 40);
		healthBar->pivot = vec2(0.0f);
		healthBar->inheritParentColor = false;
		healthBar->color = glm::vec4(0.8f, 0.1f, 0.1f, 1.0f);

		healthBarBg->AddChild(healthBar);


		Owner = dynamic_cast<Entity*>(target);

		//debuffs->origin = glm::vec2(0.5);



	}

	void Update() override
	{	
		UiVerticalBox::Update();

		healthBar->size.x = 600.0f * (Owner->Health / Owner->MaxHealth);

	}

};