#ifndef LEVELOBJECTFACTORY_H
#define LEVELOBJECTFACTORY_H

#include "Entity.hpp"
#include <map>
#include <string>

using CreateEntityFn = Entity * (*)();

class LevelObjectFactory {
public:
    static LevelObjectFactory& instance();

    void registerType(const std::string& technicalName, CreateEntityFn func);

    bool isRegistered(const std::string& technicalName) const;

    Entity* create(const std::string& technicalName) const;

private:
    LevelObjectFactory() = default;
    ~LevelObjectFactory() = default;
    LevelObjectFactory(const LevelObjectFactory&) = delete;
    LevelObjectFactory& operator=(const LevelObjectFactory&) = delete;

    std::map<std::string, CreateEntityFn> registry_;
};

#endif // LEVELOBJECTFACTORY_H
