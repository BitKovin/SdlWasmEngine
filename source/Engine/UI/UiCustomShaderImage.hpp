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
    std::unordered_map<std::string, vec4>                vec4Final;

public:
    std::unordered_map<std::string, bgfx::TextureHandle> Textures;
    // No separate float/"scalar" map — bgfx uniforms are vec4-sized no
    // matter what you conceptually pass, so a scalar just goes in Vector4s
    // as vec4(v, 0, 0, 0).
    std::unordered_map<std::string, vec4>                Vector4s;

    UiCustomShaderImage() {}
    ~UiCustomShaderImage() {}

    void FinalizeChildren() override
    {
        texturesFinal = Textures;
        vec4Final     = Vector4s;
        UiElement::FinalizeChildren();
    }

    void Draw() override
    {
        // Effects win over PixelShader — same rule as UiImage/UiButton/
        // UiVideo/UiTextBox (see UiElement.h). Worth calling out specifically
        // here: if PixelShader *is* the whole visual for your shader (say, a
        // composite background+fill effect), turning on shadow/outline/glow
        // replaces that visual outright rather than layering on top of it —
        // this fits a "single texture + filter" shader far better than a
        // composite one. If that's not what you want, leave shadow/outline/
        // glow off on that element.
        //
        // u_TextureSize isn't set automatically: this class only stores raw
        // bgfx::TextureHandle values, not the width/height a Texture* would
        // give us. Add it to Vector4s yourself if your effects shader needs
        // it (see UiImage::Draw for the pattern).
        DrawSelfTexturedParams(texturesFinal, vec4Final, GetFinalColor(), PixelShader);
        UiElement::Draw();
    }
};
