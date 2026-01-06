#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include "../Level.hpp"

#include <unordered_map>

class UiCustomShaderImage : public UiElement
{

private:

	std::unordered_map<std::string, GLuint> texturesFinal;
	std::unordered_map<std::string, float> scalarsFinal;

public:


	std::unordered_map<std::string, GLuint> Textures;
	std::unordered_map<std::string, float> Scalars;

	UiCustomShaderImage()
	{



	}

	~UiCustomShaderImage()
	{

	}

	void FinalizeChildren() override
	{
		texturesFinal = Textures;
		scalarsFinal = Scalars;
		UiElement::FinalizeChildren();
	}

	void Draw()
	{

		vec2 pos = finalizedPosition + finalizedOffset;


		if (PixelShader.empty())
		{
			//UiRenderer::DrawTexturedRect(pos, finalizedSize, rotation, pivot, tex->getID(), GetFinalColor());
		}
		else
		{
			UiRenderer::DrawTexturedRectShaderParams(pos, finalizedSize, rotation, pivot, texturesFinal, scalarsFinal, GetFinalColor(), PixelShader);
		}


		UiElement::Draw();
	}

private:


};