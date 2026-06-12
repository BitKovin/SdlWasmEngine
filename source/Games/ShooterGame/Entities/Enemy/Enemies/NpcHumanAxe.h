#pragma once

#include "NpcHumanBase.h"

class NpcHumanAxe : public NpcHumanBase
{
private:
    bool attacking = false;
    bool attackingDamage = false;

    void UpdateAttackDamage();

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;

    void Attack() override;

    void LoadAssets() override;

public:
    NpcHumanAxe();

    void Stun() override;

    void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr) override;

    void AsyncUpdate() override;

    void Serialize(json& target) override;

    void Deserialize(json& source) override;

    // ── Replication – pass axe-specific state on top of base snapshot ────
    void NetSerialize(NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;
};
