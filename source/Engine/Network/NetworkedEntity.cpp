#include "NetworkedEntity.h"
#include "NetworkManager.h"

void NetworkedEntity::PushNetworkUpdate(float /*dt*/) {
    if (!NetworkManager::IsActive()) return;
    // Rate control lives in NetworkManager::Tick.  This call just enqueues
    // the current state; it is safe to call every frame.
    NetworkManager::EnqueueEntityUpdate(this);
}


void NetworkedEntity::SendRPC(uint8_t rpcId, NetPacket& args, RPCTarget target) {
    if (!NetworkManager::IsActive())
    {
        if (target == RPCTarget::All || target == RPCTarget::Server)
        {
            OnRPC(rpcId, args);
        }

        return;
    }
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
