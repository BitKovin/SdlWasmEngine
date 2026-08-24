#include <Ecs/ISystem.h>
#include "EcsPlayerTestSystems.h"
#include <Logger.hpp>


TICK_ENTITY(PlayerTestTickSystem, PostUpdate, PlayerTestComponent, self, component)
{
	Logger::Log(self->Id);
}

REGISTER_TICK(PlayerTestTickSystem)

EVENT_HANDLER(PlayerJumpEventHandle, PlayerJumpEvent)
{
	Logger::Log("jump height: " + to_string(e.jumpHeight));
}