// EntityHandle.cpp
#include "EntityHandle.h"
#include <Network/NetworkedEntity.h>
#include <Network/NetworkManager.h>
#include <Level.hpp>

Entity* EntityHandle::Resolve(bool resolveProxy)
{
    if (!IsValid()) return nullptr;

    if (isNetworked)
    {

        auto ne = NetworkManager::Find(networkId);

		if (ne && ne->networkId == networkId)
		{
            if (resolveProxy && ne->LocalProxy != nullptr)
            {

                if (ne->LocalProxy)
                    entityId = ne->LocalProxy->Id;

                return ne->LocalProxy;
            }
			return ne;
		}
        
        return nullptr;
    }
    else
    {
        return Level::Current->FindEntityWithId(entityId);
    }
}

uint8_t EntityHandle::GetOwnerPeerId() const
{
    if (!isNetworked) return 0;

	auto ne = NetworkManager::Find(networkId);

	if (ne && ne->networkId == networkId)
		return ne->networkOwner;

    return 0;
}
