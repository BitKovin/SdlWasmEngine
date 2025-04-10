#include "LevelObjectFactory.h"

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
        return (it->second)();
    return nullptr;
}
