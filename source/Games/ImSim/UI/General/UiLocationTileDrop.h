#pragma once
#include <UI/UiText.hpp>
#include <UI/UiCanvas.hpp>
#include <memory>

class UiLocationTileDrop : public UiCanvas
{
public:
	UiLocationTileDrop(std::string title);

	void Update() override;

	static void PlayTitleCard(std::string title);

private:

	std::shared_ptr<UiText> text;

	float time = 0;

	float duration = 5.6;
	float blendIn = 0.2f;
	float blendOut = 1.0f;

	float GetBlendValue(float time) const;

};



