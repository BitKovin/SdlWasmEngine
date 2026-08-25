#pragma once
// ComponentRegistry.h
//
// Static, no-instance registry of every ECS component type known to the
// engine - every call is ComponentRegistry::Something(...), no ::Get()
// anywhere. Register a component with REGISTER_COMPONENT(Type) once,
// anywhere (a .cpp is fine), and it becomes constructible-with-defaults,
// introspectable, and cloneable everywhere else in the engine without any
// further wiring.
//
// REGISTER_COMPONENT is safe to put directly in a header, even one included
// by many .cpp files: the generated registrar is a C++17 inline variable,
// not a plain static one, so the linker collapses every translation unit's
// copy into a single instance instead of registering once per include.
//
// NOTE: REGISTER_COMPONENT (and ECS_COMPONENT in Entity.h) must be invoked
// at namespace/class scope, never inside a function body.

#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

struct ComponentTypeInfo
{
    entt::id_type Id = 0;
    std::string   Name;
    std::function<void(entt::registry&, entt::entity)>               EmplaceDefault;
    std::function<bool(const entt::registry&, entt::entity)>         Has;
    std::function<void(entt::registry&, entt::entity)>               Remove;
    std::function<void(entt::registry&, entt::entity, entt::entity)> CopyTo; // src -> dst, no-op if src lacks it
};

class ComponentRegistry
{
public:
    ComponentRegistry() = delete;

    template<typename T>
    static void Register(const char* name)
    {
        static_assert(std::is_copy_constructible_v<T>,
            "REGISTER_COMPONENT requires a copy-constructible type so entity cloning stays "
            "generic. If this component genuinely can't be copied (e.g. it owns a unique_ptr "
            "resource handle), that's a real decision about what cloning means for it - don't "
            "register it, and handle it explicitly wherever entities are cloned.");

        ComponentTypeInfo info;
        info.Id   = entt::type_id<T>().hash();
        info.Name = name;
        info.EmplaceDefault = [](entt::registry& r, entt::entity e) { r.emplace_or_replace<T>(e); };
        info.Has            = [](const entt::registry& r, entt::entity e) { return r.all_of<T>(e); };
        info.Remove         = [](entt::registry& r, entt::entity e) { r.remove<T>(e); };
        info.CopyTo         = [](entt::registry& r, entt::entity src, entt::entity dst)
        {
            if (!r.all_of<T>(src)) return;
            // entt optimizes empty/tag components (no data members) into
            // storage that only tracks presence, not a value - get<T>() on
            // one of those isn't usable the way it is for a normal
            // component, so there's nothing to copy: presence is the only
            // state a tag component has, and emplace alone reproduces it.
            if constexpr (std::is_empty_v<T>)
                r.emplace_or_replace<T>(dst);
            else
                r.emplace_or_replace<T>(dst, r.get<T>(src));
        };

        Storage().ByName[name]  = info.Id;
        Storage().ById[info.Id] = std::move(info);
    }

    static const ComponentTypeInfo* Find(const std::string& name)
    {
        auto& byName = Storage().ByName;
        auto it = byName.find(name);
        if (it == byName.end()) return nullptr;
        return &Storage().ById.at(it->second);
    }

    static const std::unordered_map<entt::id_type, ComponentTypeInfo>& All() { return Storage().ById; }

    // Clones every REGISTERED component from src onto a freshly created entity.
    // Does NOT run any Entity subclass constructor, does NOT attach an
    // EntityOwner, and does NOT copy non-ECS state still living as plain
    // members on LevelObject/Entity (Position, Health, etc.).
    //
    // The result is NOT a real engine entity: it has no rendering, no
    // LevelObject lifecycle, and EcsWorld::GetEntity() on it returns
    // nullptr. Only use this for purely data-driven, non-rendered rows -
    // for anything a player can see, spawn it through Entity::Spawn()
    // instead, the same as any other entity.
    static entt::entity Clone(entt::registry& r, entt::entity src)
    {
        entt::entity dst = r.create();
        for (auto& [id, info] : Storage().ById)
            info.CopyTo(r, src, dst);
        return dst;
    }

private:
    struct Data
    {
        std::unordered_map<entt::id_type, ComponentTypeInfo> ById;
        std::unordered_map<std::string, entt::id_type>       ByName;
    };
    static Data& Storage() { static Data data; return data; }
};

// EcsDetail holds only generated registrar plumbing - never reference it
// directly, it's not part of the public API and its contents can change.
#define REGISTER_COMPONENT(Type) \
    namespace EcsDetail { \
        struct Type##_ComponentRegistrar \
        { \
            Type##_ComponentRegistrar() { ComponentRegistry::Register<Type>(#Type); } \
        }; \
        inline Type##_ComponentRegistrar Type##_componentRegistrar_instance; \
    }

// Manual alternative to REGISTER_COMPONENT - use when this code may live in
// a shared library (DLL/.so). See the "SHARED LIBRARIES" note in Ecs.h for
// why static-initializer registration can silently fail to run there.
// Same call, no static object at all - call it yourself, from a function
// you control, before EcsScheduler::Finalize():
//   REGISTER_COMPONENT_MANUAL(Health);
#define REGISTER_COMPONENT_MANUAL(Type) ComponentRegistry::Register<Type>(#Type)
