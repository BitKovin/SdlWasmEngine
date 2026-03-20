#pragma once
#include <string_view>
#include <array>
#include <functional>
#include <iosfwd>   // forward-declare ostream without pulling in <iostream>

// ─────────────────────────────────────────────
//  Combo Tier Score System
//  Tiers: None → Bronze → Silver → Gold → Platinum → Diamond
//
//  - Scoring fills the current tier's progress bar.
//  - When the bar fills (>=1.0), the player advances one tier.
//  - Taking damage drains progress proportionally to damage dealt.
//  - When progress drops below 0.0, the player loses one tier.
//  - Each tier applies a score multiplier to all base score gains.
// ─────────────────────────────────────────────

enum class ComboTier : int {
    None     = 0,
    Bronze   = 1,
    Silver   = 2,
    Gold     = 3,
    Platinum = 4,
    //Diamond  = 5,

    COUNT
};

// ── Per-tier configuration ────────────────────
struct TierConfig {
    std::string_view name;
    double           scoreMultiplier;  // Applied to every base score gain
    double           scoreToAdvance;   // Base score needed to fill the bar (0 = max tier)
    double           damageToDeplete;  // Damage required to fully drain the bar (0 = immune)
};

static constexpr int TIER_COUNT = static_cast<int>(ComboTier::COUNT);

// Edit these values to tune feel:
//   damageToDeplete = total damage that wipes the bar from 100% → 0%
constexpr std::array<TierConfig, TIER_COUNT> TIER_CONFIGS = {{
                //   name      multplier    points to progress    damage to fully regress tier
    /* None     */ { "x1",      1.0,          150.0,                      0.0 }, 
    /* Bronze   */ { "x2",    2,          250.0,                    40.0 },
    /* Silver   */ { "x4",    4,          400.0,                    35.0 },
    /* Gold     */ { "x8",      8,          600.0,                    25.0 },
    /* Platinum */ { "x16",  16,          500.0,                     15.0 },
    //  /* Diamond  */ { "Diamond",   4.0,          100.0,                     15.0 }, 
}};

// ── ScoreSystem ───────────────────────────────
class ScoreSystem {
public:
    using TierChangedCallback = std::function<void(ComboTier oldTier, ComboTier newTier)>;

    ScoreSystem();

    // Earn points. Each tier boundary is crossed at its own multiplier.
    // Returns actual score added after multipliers.
    double addScore(double baseScore);

    // Take damage. Each tier boundary is crossed at its own damageToDeplete.
    void takeDamage(double damage);

    // Optional callback fired on every tier change.
    void onTierChanged(TierChangedCallback cb);

    // Accessors
    double           totalScore()   const;
    ComboTier        tier()         const;
    double           tierProgress() const;  // 0.0 – 1.0
    double           multiplier()   const;
    std::string_view tierName()     const;

    // Debug print
    void printStatus(std::ostream& os) const;

    static ScoreSystem& Instance();

private:
    double              totalScore_;
    double              tierProgress_;  // 0.0 – 1.0
    ComboTier           currentTier_;
    TierChangedCallback tierChangedCb_;

    const TierConfig& currentConfig() const;
    void promoteTier();
    void demoteTier();
    static void printBar(std::ostream& os, double progress, int width);
};
