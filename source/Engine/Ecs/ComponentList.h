#pragma once
// ComponentList.h
//
// Per-entity, cached list of which components an entity actually holds.
// Populated once at construction time by ECS_COMPONENT (see Entity.h), so an
// inspector/debug panel can render "what does this entity have" by reading
// entity.Components().All() without touching entt internals or walking every
// registered component type against the entity.

#include <entt/entt.hpp>
#include <algorithm>
#include <vector>

class ComponentList
{
public:
    struct Entry
    {
        entt::id_type Id;
        const char*   Name;
    };

    void Add(entt::id_type id, const char* name) { m_Entries.push_back({ id, name }); }

    bool Has(entt::id_type id) const
    {
        return std::any_of(m_Entries.begin(), m_Entries.end(),
                            [id](const Entry& e) { return e.Id == id; });
    }

    const std::vector<Entry>& All() const { return m_Entries; }

private:
    std::vector<Entry> m_Entries;
};
