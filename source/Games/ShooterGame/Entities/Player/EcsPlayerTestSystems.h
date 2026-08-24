#pragma once

#include <Ecs/Ecs.h>

struct PlayerTestComponent
{
	float data;
};

REGISTER_COMPONENT(PlayerTestComponent)

struct PlayerJumpEvent
{
	float jumpHeight = 0;
};
REGISTER_EVENT	(PlayerJumpEvent)