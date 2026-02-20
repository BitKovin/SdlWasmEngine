#pragma once
#include "UiElement.h"

class UiCanvas : public UiElement
{
public:
	
	bool ScaleToParent = true;

	vec2 GetSize()
	{


		if (parent == nullptr || ScaleToParent == false)
			return size;

		return parent->GetSize();
	}

	void Update()
	{
		
		size = GetSize();

		UiElement::Update();
	}

private:

};