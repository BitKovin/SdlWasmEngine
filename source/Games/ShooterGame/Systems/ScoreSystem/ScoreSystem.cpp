#include "ScoreSystem.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

// ── Construction ─────────────────────────────
ScoreSystem::ScoreSystem()
    : totalScore_(0.0)
    , tierProgress_(0.0)
    , currentTier_(ComboTier::None)
{}

// ── Public ───────────────────────────────────
double ScoreSystem::addScore(double baseScore) {
    if (baseScore <= 0.0) return 0.0;

    double earned    = 0.0;
    double remaining = baseScore;

    while (remaining > 0.0) {
        const TierConfig& cfg = currentConfig();

        if (cfg.scoreToAdvance <= 0.0) {
            // Max tier (Diamond): absorb all remaining at its multiplier, no bar
            earned   += remaining * cfg.scoreMultiplier;
            remaining = 0.0;
            break;
        }

        // Raw score needed to fill the rest of this tier's bar
        const double rawToFill = (1.0 - tierProgress_) * cfg.scoreToAdvance;

        if (remaining < rawToFill) {
            earned        += remaining * cfg.scoreMultiplier;
            tierProgress_ += remaining / cfg.scoreToAdvance;
            remaining      = 0.0;
        } else 
        {

            earned += rawToFill * cfg.scoreMultiplier;

            if ((int)currentTier_ == (int)ComboTier::COUNT - 1)
            {
                tierProgress_ = 1.0f;
                remaining = 0;
            }
            else
            {          
                // Exactly fill this tier, promote, then continue with leftover
                remaining    -= rawToFill;
                tierProgress_ = 0.0;
                promoteTier();
            }
        }
    }

    totalScore_ += earned;
    return earned;
}

void ScoreSystem::takeDamage(double damage) {
    if (damage <= 0.0 || currentTier_ == ComboTier::None) return;

    double remaining = damage;

    while (remaining > 0.0 && currentTier_ != ComboTier::None) {
        const TierConfig& cfg = currentConfig();
        if (cfg.damageToDeplete <= 0.0) break; // immune tier

        // Raw damage needed to empty the bar from its current fill level
        const double rawToEmpty = tierProgress_ * cfg.damageToDeplete;

        if (remaining <= rawToEmpty) {
            tierProgress_ -= remaining / cfg.damageToDeplete;
            remaining      = 0.0;
        } else {
            // Drain this tier completely, demote, arrive at a full bar
            remaining    -= rawToEmpty;
            tierProgress_  = 0.0;
            demoteTier();
            tierProgress_  = 1.0;
        }
    }

    if (currentTier_ == ComboTier::None)
        tierProgress_ = 0.0;
}

void ScoreSystem::onTierChanged(TierChangedCallback cb) {
    tierChangedCb_ = std::move(cb);
}

double           ScoreSystem::totalScore()   const { return totalScore_; }
ComboTier        ScoreSystem::tier()         const { return currentTier_; }
double           ScoreSystem::tierProgress() const { return tierProgress_; }
double           ScoreSystem::multiplier()   const { return currentConfig().scoreMultiplier; }
std::string_view ScoreSystem::tierName()     const { return currentConfig().name; }

void ScoreSystem::printStatus(std::ostream& os) const {
    os << "┌──────────────────────────────────┐\n";
    os << "│  Score  : " << std::setw(8) << std::fixed << std::setprecision(1)
       << totalScore_ << "                    │\n";
    os << "│  Tier   : " << std::left << std::setw(9) << tierName()
       << "  ×" << std::fixed << std::setprecision(1)
       << multiplier() << "              │\n";
    os << "│  Bar    : [";
    printBar(os, tierProgress_, 20);
    os << "] " << std::right << std::setw(5) << std::fixed << std::setprecision(1)
       << tierProgress_ * 100.0 << "%  │\n";
    os << "└──────────────────────────────────┘\n";
}

ScoreSystem& ScoreSystem::Instance()
{
    static ScoreSystem sys;

    return sys;
}

// ── Private ──────────────────────────────────
const TierConfig& ScoreSystem::currentConfig() const {
    return TIER_CONFIGS[static_cast<int>(currentTier_)];
}

void ScoreSystem::promoteTier() {
    ComboTier old = currentTier_;
    currentTier_  = static_cast<ComboTier>(static_cast<int>(currentTier_) + 1);
    if (tierChangedCb_) tierChangedCb_(old, currentTier_);
}

void ScoreSystem::demoteTier() {
    ComboTier old = currentTier_;
    currentTier_  = static_cast<ComboTier>(static_cast<int>(currentTier_) - 1);
    if (tierChangedCb_) tierChangedCb_(old, currentTier_);
}

void ScoreSystem::printBar(std::ostream& os, double progress, int width) {
    int filled = static_cast<int>(std::clamp(progress, 0.0, 1.0) * width);
    for (int i = 0; i < width; ++i)
        os << (i < filled ? '#' : '.');
}
