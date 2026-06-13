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

void NetworkedEntity::SetMeAsOwner() {
    if (!CanMigrateOwner) return;
    SetOwner(NetworkManager::GetLocalPeerId());
}

void NetworkedEntity::SetOwner(uint8_t peerId) {
    if (!CanMigrateOwner) return;
    networkOwner = peerId;
    isOwned      = (peerId == NetworkManager::GetLocalPeerId());
    if (NetworkManager::IsActive()) {
        NetworkManager::BroadcastOwnerChange(networkId, peerId);
    }
}
