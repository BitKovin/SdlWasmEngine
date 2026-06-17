// TestNpc.hpp
#pragma once

#include "NpcHumanBase.h"

class TestNpc : public NpcHumanBase
{
private:
    // attackingDamage is a sub-flag of the Attacking state:
    // true while the hit-detection window is open inside the bite animation.
    bool attackingDamage = false;

    void UpdateAttackDamage();

protected:
    void ProcessAnimationEvent(AnimationEvent& event) override;

    void Attack() override;

    void LoadAssets() override; 

public:
    TestNpc();

    void Stun() override;

    void OnDamage(float Damage, Entity* DamageCauser = nullptr,
                  Entity* Weapon = nullptr) override;

    void AsyncUpdate() override;

    void Serialize(json& target)   override;
    void Deserialize(json& source) override;

    // ── Replication ──────────────────────────────────────────────────────
    // Dog-specific fields are appended after the base snapshot.
    void NetSerialize  (NetPacket& packet) override;
    void NetDeserialize(NetPacket& packet) override;
};
