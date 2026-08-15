#include "LevelObjectFactory.h"

#include "malloc_override.h"

LevelObjectFactory& LevelObjectFactory::instance() {
    static LevelObjectFactory factory;
    return factory;
}

void LevelObjectFactory::registerType(const std::string& technicalName, CreateEntityFn func) {
    if (!isRegistered(technicalName)) {
        registry_[technicalName] = func;
    }
}

bool LevelObjectFactory::isRegistered(const std::string& technicalName) const {
    return registry_.find(technicalName) != registry_.end();
}

Entity* LevelObjectFactory::create(const std::string& technicalName) const 
{

    auto it = registry_.find(technicalName);
    if (it != registry_.end()) 
    {
        Entity* ent = (it->second)();

        if (ent)
        {
            ent->ClassName = technicalName;
        }
        return ent;

    }
    return nullptr;
}

#include "Entities/WorldSpawn.h"
#include "Entities/StaticMeshEntity.h"
#include "Entities/Spawner.h"
#include "Entities/Editor/Counter.h"
#include "Entities/Editor/LogicOnce.h"
#include "Entities/transformPoint.h"
#include "Entities/Brushes/DestructibleBrush.hpp"
#include "Entities/Brushes/MovebleBrush.hpp"
#include "Entities/Brushes/TriggerBase.hpp"
#include "Entities/Brushes/TriggerOnce.hpp"
#include "Entities/Brushes/vis_through.hpp"
#include "Entities/Brushes/DynamicPhysicsBrush.hpp"
#include <Entities/Brushes/Sound/FmodReverbZone.h>
#include <Entities/Brushes/TriggerDamage.hpp>

void LevelObjectFactory::registerDefaults()
{
    REGISTER_ENTITY_INLINE("static_mesh", StaticMeshEntity);
    REGISTER_ENTITY_INLINE("worldspawn", WorldSpawn);
    REGISTER_ENTITY_INLINE("spawner", Spawner);
    REGISTER_ENTITY_INLINE("counter", Counter);
    REGISTER_ENTITY_INLINE("logic_once", LogicOnce);
    REGISTER_ENTITY_INLINE("transformPoint", TransformPoint);
    REGISTER_ENTITY_INLINE("destructible", DestructibleBrush);
    REGISTER_ENTITY_INLINE("func_door", MovebleBrush);
    REGISTER_ENTITY_INLINE("trigger", TriggerBase);
    REGISTER_ENTITY_INLINE("triggerOnce", TriggerOnce);
    REGISTER_ENTITY_INLINE("vis_through", vis_through);
    REGISTER_ENTITY_INLINE("func_physics_dynamic", DynamicPhysicsBrush);
	REGISTER_ENTITY_INLINE("snd_reverb_zone", FmodReverbZone);
	REGISTER_ENTITY_INLINE("trigger_damage", TriggerDamage);
}
