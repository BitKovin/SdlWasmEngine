#pragma once

#include <vector>
#include <tuple>
#include "EObject.hpp"
#include "LevelObject.hpp"

#include "glm.h"

using namespace std;

template<typename... Components>
class Entity : public LevelObject
{
public:
    Entity() = default;
    Entity(Components&&... components)
        : m_components(std::forward<Components>(components)...) {}

    virtual ~Entity() = default;
    virtual void Destroy() {}

    
    template<typename T>
    T& get()
    {
        return std::get<T>(m_components);
    }

    template<typename T>
    const T& get() const
    {
        return std::get<T>(m_components);
    }

private:
    std::tuple<Components...> m_components;
};
