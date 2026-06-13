// NpcHumanAxe.h
#pragma once

#include "NpcHumanBase.h"

class NpcHumanAxe : public NpcHumanBase
{
private:
    // attackingDamage is a sub-flag of the Attacking state:
    // true while the hit-detection window is open inside an attack animation.
    bool attackingDamage = false;

    void UpdateAttackDamage();

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;

    void Attack() override;

    void LoadAssets() override;

public:
    NpcHumanAxe();

    void Stun() override;

    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr) override;

    void AsyncUpdate() override;

    void Serialize(json& target)   override;
    void Deserialize(json& source) override;

    // ── Replication ──────────────────────────────────────────────────────
    // Axe-specific fields are appended after the base snapshot.
    void NetSerialize  (NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;
};
