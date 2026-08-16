#pragma once

#include <Entity.h>

class FuncCharacterBlock : public Entity
{
public:
	FuncCharacterBlock()
	{
		DefaultBrushCollisionMask = BodyType::CharacterCapsule;
		DefaultBrushGroup = BodyType::CharacterBlocker;
		Visible = false;
		Static = true;
	}

private:

};

