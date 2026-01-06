#pragma once
#include "UiElement.h"

class UiCanvas : public UiElement
{
public:
	
	vec2 GetSize()
	{

		if (parent == nullptr)
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