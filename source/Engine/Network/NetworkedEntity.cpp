#include "NetworkedEntity.h"
#include "NetworkManager.h"

void NetworkedEntity::PushNetworkUpdate(float /*dt*/) {
    if (!NetworkManager::IsActive()) return;
    // Rate control lives in NetworkManager::Tick.  This call just enqueues
    // the current state; it is safe to call every frame.
    NetworkManager::EnqueueEntityUpdate(this);
}

void NetworkedEntity::Destroy()
{

    if (isOwned == false) return;

    Entity::Destroy();

}

void NetworkedEntity::SendRPC(uint8_t rpcId, NetPacket& args, RPCTarget target) {
    if (!NetworkManager::IsActive()) return;
    NetworkManager::SendRPC(networkId, rpcId, args, target);
}
