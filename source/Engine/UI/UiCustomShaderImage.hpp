#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include "../Level.hpp"

#include <unordered_map>

class UiCustomShaderImage : public UiElement
{

protected:

	std::unordered_map<std::string, bgfx::TextureHandle> texturesFinal;
	std::unordered_map<std::string, float> scalarsFinal;
	std::unordered_map<std::string, vec4> vec4Final;

public:


	std::unordered_map<std::string, bgfx::TextureHandle> Textures;
	std::unordered_map<std::string, float> Scalars;
	std::unordered_map<std::string, vec4> Vector4s;

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
		vec4Final = Vector4s;
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
			UiRenderer::DrawTexturedRectShaderParams(pos, finalizedSize, rotation, pivot, texturesFinal, scalarsFinal,vec4Final, GetFinalColor(), PixelShader);
		}


		UiElement::Draw();
	}

private:


};