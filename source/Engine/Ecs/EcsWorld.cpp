#include "EcsWorld.h"
#include <Entity.h>
void EcsWorld::Destroy(entt::entity e)
{
    if (Entity* owner = GetEntity(e))
        owner->Destroy();
    else if (Storage().valid(e))
        Storage().destroy(e);
}