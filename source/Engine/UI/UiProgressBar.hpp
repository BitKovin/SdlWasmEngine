#pragma once

#include "UiCustomShaderImage.hpp"

class UiProgressBar : public UiCustomShaderImage
{
public:

	UiProgressBar()
	{
		PixelShader = "ui/progressBar";
	}

	std::string BackgroundImage = "GameData/cat.png";
	std::string ProgressImage = "GameData/cat.png";
	float Progress = 0;
	vec4 BackgroundColor = vec4(1);

	void FinalizeChildren() override
	{
		UiCustomShaderImage::FinalizeChildren();

		progressFinal = Progress;

	}

	void Draw() override
	{

		vec4Final["bgColor"] = BackgroundColor;
		texturesFinal["bgTexture"] = AssetRegistry::GetTextureFromFile(BackgroundImage)->getHandle();
		vec4Final["u_Color"] = color;
		texturesFinal["u_Texture"] = AssetRegistry::GetTextureFromFile(ProgressImage)->getHandle();
		scalarsFinal["progress"] = progressFinal;

		UiCustomShaderImage::Draw();

	}


private:

	float progressFinal = 0;


};
