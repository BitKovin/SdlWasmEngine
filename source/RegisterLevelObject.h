#ifndef REGISTER_LEVELOBJECT_H
#define REGISTER_LEVELOBJECT_H

#include "LevelObjectFactory.h"

#define REGISTER_LEVEL_OBJECT(CLASS_NAME, TECH_NAME)                 \
namespace {                                                          \
    Entity* create##CLASS_NAME() {                                   \
        return new CLASS_NAME();                                     \
    }                                                                \
    struct CLASS_NAME##Registrator {                                 \
        CLASS_NAME##Registrator() {                                  \
            auto& factory = LevelObjectFactory::instance();          \
            if (!factory.isRegistered(TECH_NAME)) {                  \
                factory.registerType(TECH_NAME, create##CLASS_NAME); \
            }                                                        \
        }                                                            \
    };                                                               \
    static CLASS_NAME##Registrator global_##CLASS_NAME##Registrator; \
}

#endif // REGISTER_LEVELOBJECT_H
