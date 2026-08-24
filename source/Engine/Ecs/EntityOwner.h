#pragma once
// EntityOwner.h
//
// Internal backlink from an entt::entity id to the owning engine Entity
// object. Every Entity attaches one to itself, once, in its constructor -
// see Entity.h. Not REGISTER_COMPONENT'd: it's not something a clone or a
// data-only registry entity should ever carry, only a real Entity object.
//
// Split into its own header (rather than living in Entity.h or EcsWorld.h)
// because both of those need it, and including either from the other would
// be circular.

class Entity;

struct EntityOwner
{
    Entity* Owner = nullptr;
};
