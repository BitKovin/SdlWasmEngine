#pragma once
// ComponentRef.h
//
// A reference to a single component on a single entity that stays valid
// across frames and tick-group boundaries. Unlike a raw T*, it never goes
// stale when an unrelated emplace/remove elsewhere reallocates storage - it
// re-resolves through the registry on every dereference instead of caching
// a pointer.
//
// This does NOT add thread safety by itself. Two ComponentRef<T> to the same
// entity from two threads race exactly like registry.get<T>() would - the
// ParallelSafe()/RunsConcurrentlyWith() discipline on ISystem still applies.

#include <entt/entt.hpp>

template<typename T>
class ComponentRef
{
public:
    ComponentRef() = default;
    ComponentRef(entt::registry& registry, entt::entity entity)
        : m_Registry(&registry), m_Entity(entity) {}

    T* operator->() const { return &m_Registry->get<T>(m_Entity); }
    T& operator*()  const { return  m_Registry->get<T>(m_Entity); }

    // Use like an optional before dereferencing: if (auto h = entity->GetHealth()) h->Current -= dmg;
    explicit operator bool() const
    {
        return m_Registry != nullptr
            && m_Registry->valid(m_Entity)
            && m_Registry->all_of<T>(m_Entity);
    }

    void Remove() const { if (*this) m_Registry->remove<T>(m_Entity); }

    entt::entity    Entity()   const { return m_Entity; }
    entt::registry* Registry() const { return m_Registry; }

private:
    entt::registry* m_Registry = nullptr;
    entt::entity    m_Entity   = entt::null;
};
