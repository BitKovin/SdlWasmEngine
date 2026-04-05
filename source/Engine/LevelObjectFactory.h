#ifndef LEVELOBJECTFACTORY_H
#define LEVELOBJECTFACTORY_H

#include "Entity.h"
#include <map>
#include <string>

using CreateEntityFn = Entity * (*)();

class LevelObjectFactory {
public:
    static LevelObjectFactory& instance();

    void registerDefaults();

    void registerType(const std::string& technicalName, CreateEntityFn func);

    bool isRegistered(const std::string& technicalName) const;

    Entity* create(const std::string& technicalName) const;

	std::map<std::string, CreateEntityFn> GetRegistry() const { return registry_; }

private:
    LevelObjectFactory() = default;
    ~LevelObjectFactory() = default;
    LevelObjectFactory(const LevelObjectFactory&) = delete;
    LevelObjectFactory& operator=(const LevelObjectFactory&) = delete;

    std::map<std::string, CreateEntityFn> registry_;
};

#define REGISTER_ENTITY_INLINE(NAME, TYPE) \
    LevelObjectFactory::instance().registerType(NAME, []() -> Entity* { return new TYPE(); })


#endif // LEVELOBJECTFACTORY_H
