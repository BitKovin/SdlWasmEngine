#pragma once

// PhysicsMaterialHelper
// ---------------------
// Classifies an engine texture name/path into a PhysicsMaterialType.
//
// Each individual material is a single set bit, so gameplay code that
// only cares about a family of materials (footstep sounds, impact
// decals/particles, bullet penetration, ...) can test against one of the
// PhysicsMaterialType::Group_* values instead of enumerating every
// member of that family:
//
//     PhysicsMaterialType type = PhysicsMaterialHelper::Classify(texturePath);
//     if (PhysicsMaterialHelper::IsInGroup(type, PhysicsMaterialType::Group_Metal))
//         PlayFootstepSound(kMetalFootstepBank);
//
// This is a C++ port of a C texture-name classifier that was built on the
// Remimu regex engine (https://github.com/wareya/Remimu, single-header,
// CC0, "remimu.h"). The classification LOGIC (rule set, rule order,
// suffix/prefix stripping, folder fallback) is unchanged from the C
// original; only the surrounding structure was translated to modern C++.

#include <cstdint>
#include <string>
#include <string_view>

// ---------------------------------------------------------------------
// Bit positions for each leaf material. Not meant to be used directly -
// this only exists so PhysicsMaterialType's enumerators (including the
// Group_* combinations) can be defined below via plain integer
// bitwise-OR. A scoped enum has no built-in "|" operator and can't use
// one of its own not-yet-declared operator overloads while it's still
// being defined, so the OR-ing happens here, on plain uint64_t, instead.
// ---------------------------------------------------------------------
namespace PhysicsMaterialTypeBits
{
    constexpr uint64_t Utility    = 1ULL << 0;
    constexpr uint64_t Dev        = 1ULL << 1;
    constexpr uint64_t Sky        = 1ULL << 2;
    constexpr uint64_t PropMisc   = 1ULL << 3;

    constexpr uint64_t Stone      = 1ULL << 4;
    constexpr uint64_t Rock       = 1ULL << 5;
    constexpr uint64_t Brick      = 1ULL << 6;
    constexpr uint64_t Concrete   = 1ULL << 7;
    constexpr uint64_t Marble     = 1ULL << 8;
    constexpr uint64_t Tile       = 1ULL << 9;
    constexpr uint64_t Plaster    = 1ULL << 10;

    constexpr uint64_t Metal      = 1ULL << 11;
    constexpr uint64_t MetalGrate = 1ULL << 12;

    constexpr uint64_t Wood       = 1ULL << 13;

    constexpr uint64_t Dirt       = 1ULL << 14;
    constexpr uint64_t Grass      = 1ULL << 15;
    constexpr uint64_t Sand       = 1ULL << 16;
    constexpr uint64_t Gravel     = 1ULL << 17;
    constexpr uint64_t Asphalt    = 1ULL << 18;
    constexpr uint64_t Snow       = 1ULL << 19;
    constexpr uint64_t Ice        = 1ULL << 20;
    constexpr uint64_t Foliage    = 1ULL << 21;

    constexpr uint64_t Water      = 1ULL << 22;
    constexpr uint64_t Lava       = 1ULL << 23;
    constexpr uint64_t Slime      = 1ULL << 24;

    constexpr uint64_t Blood      = 1ULL << 25;
    constexpr uint64_t Flesh      = 1ULL << 26;
    constexpr uint64_t Bone       = 1ULL << 27;

    constexpr uint64_t Glass      = 1ULL << 28;
    constexpr uint64_t Mirror     = 1ULL << 29;
    constexpr uint64_t Fabric     = 1ULL << 30;
    constexpr uint64_t Carpet     = 1ULL << 31;
    constexpr uint64_t Paper      = 1ULL << 32;
    constexpr uint64_t Cardboard  = 1ULL << 33;
    constexpr uint64_t Plastic    = 1ULL << 34;
    constexpr uint64_t Rubber     = 1ULL << 35;
    constexpr uint64_t Rope       = 1ULL << 36;

    constexpr size_t LeafCount = 37;
}

// ---------------------------------------------------------------------
// PhysicsMaterialType
//
// Surface material classification for footstep sounds / impact decals /
// physics. Every leaf value (Stone, Metal, Wood, ...) occupies exactly
// one bit. Unknown stays 0 so an un-classified texture is easy to spot
// (and easy to default-init to, e.g. `PhysicsMaterialType type{};`).
//
// Group_* values are OR-combinations of leaf values that gameplay code
// can generalize over - see IsInGroup() below and the comments next to
// each Group_* for the reasoning behind what got merged vs. kept
// separate (e.g. brick and rock both fold into Group_Stone; snow and ice
// stay separate because they don't sound/behave alike).
// ---------------------------------------------------------------------
enum class PhysicsMaterialType : uint64_t
{
    Unknown = 0,

    // --- Non-rendering / engine-only ------------------------------------
    Utility    = PhysicsMaterialTypeBits::Utility,     // clip, trigger, skip, hint, origin, nodraw, occluder
    Dev        = PhysicsMaterialTypeBits::Dev,         // placeholder / blockout / debug-grid / editor textures
    Sky        = PhysicsMaterialTypeBits::Sky,
    PropMisc   = PhysicsMaterialTypeBits::PropMisc,    // unique prop/item skins (radio, battery, console...) that
                                                        // don't reduce to one physical surface material by name alone

    // --- Hard architectural surfaces -------------------------------------
    Stone      = PhysicsMaterialTypeBits::Stone,
    Rock       = PhysicsMaterialTypeBits::Rock,
    Brick      = PhysicsMaterialTypeBits::Brick,
    Concrete   = PhysicsMaterialTypeBits::Concrete,
    Marble     = PhysicsMaterialTypeBits::Marble,
    Tile       = PhysicsMaterialTypeBits::Tile,
    Plaster    = PhysicsMaterialTypeBits::Plaster,

    // --- Metal ------------------------------------------------------------
    Metal      = PhysicsMaterialTypeBits::Metal,
    MetalGrate = PhysicsMaterialTypeBits::MetalGrate,  // grating / mesh / chain-link - same family, different footstep

    // --- Wood ---------------------------------------------------------------
    Wood       = PhysicsMaterialTypeBits::Wood,

    // --- Ground / terrain -----------------------------------------------
    Dirt       = PhysicsMaterialTypeBits::Dirt,
    Grass      = PhysicsMaterialTypeBits::Grass,
    Sand       = PhysicsMaterialTypeBits::Sand,
    Gravel     = PhysicsMaterialTypeBits::Gravel,
    Asphalt    = PhysicsMaterialTypeBits::Asphalt,
    Snow       = PhysicsMaterialTypeBits::Snow,
    Ice        = PhysicsMaterialTypeBits::Ice,
    Foliage    = PhysicsMaterialTypeBits::Foliage,     // leaves / vines / plants

    // --- Liquids --------------------------------------------------------
    Water      = PhysicsMaterialTypeBits::Water,
    Lava       = PhysicsMaterialTypeBits::Lava,
    Slime      = PhysicsMaterialTypeBits::Slime,       // toxic sludge / acid / generic "slime"

    // --- Organic ------------------------------------------------------------
    Blood      = PhysicsMaterialTypeBits::Blood,
    Flesh      = PhysicsMaterialTypeBits::Flesh,
    Bone       = PhysicsMaterialTypeBits::Bone,

    // --- Glass / soft goods -----------------------------------------------
    Glass      = PhysicsMaterialTypeBits::Glass,
    Mirror     = PhysicsMaterialTypeBits::Mirror,
    Fabric     = PhysicsMaterialTypeBits::Fabric,
    Carpet     = PhysicsMaterialTypeBits::Carpet,
    Paper      = PhysicsMaterialTypeBits::Paper,
    Cardboard  = PhysicsMaterialTypeBits::Cardboard,
    Plastic    = PhysicsMaterialTypeBits::Plastic,
    Rubber     = PhysicsMaterialTypeBits::Rubber,
    Rope       = PhysicsMaterialTypeBits::Rope,        // rope / cable / wire

    // --- Groups: bitwise-OR combinations of the leaf materials above, for
    //     gameplay code that wants to react to a whole family at once ------
    Group_Stone   = PhysicsMaterialTypeBits::Stone | PhysicsMaterialTypeBits::Rock
                   | PhysicsMaterialTypeBits::Brick | PhysicsMaterialTypeBits::Concrete
                   | PhysicsMaterialTypeBits::Marble | PhysicsMaterialTypeBits::Plaster
                   | PhysicsMaterialTypeBits::Asphalt,  // hard masonry/paving - similar enough hard-footstep hit
    Group_Tile    = PhysicsMaterialTypeBits::Tile,       // hard but hollow/reverberant - kept apart from Stone
    Group_Metal   = PhysicsMaterialTypeBits::Metal | PhysicsMaterialTypeBits::MetalGrate,
    Group_Wood    = PhysicsMaterialTypeBits::Wood,
    Group_Dirt    = PhysicsMaterialTypeBits::Dirt,
    Group_Grass   = PhysicsMaterialTypeBits::Grass | PhysicsMaterialTypeBits::Foliage,
    Group_Sand    = PhysicsMaterialTypeBits::Sand,
    Group_Gravel  = PhysicsMaterialTypeBits::Gravel,     // loose-rock crunch - distinct from packed Dirt
    Group_Snow    = PhysicsMaterialTypeBits::Snow,       // kept apart from Ice - very different footstep feel
    Group_Ice     = PhysicsMaterialTypeBits::Ice,
    Group_Water   = PhysicsMaterialTypeBits::Water,
    Group_Lava    = PhysicsMaterialTypeBits::Lava,
    Group_Slime   = PhysicsMaterialTypeBits::Slime,
    Group_Flesh   = PhysicsMaterialTypeBits::Blood | PhysicsMaterialTypeBits::Flesh,
    Group_Bone    = PhysicsMaterialTypeBits::Bone,
    Group_Glass   = PhysicsMaterialTypeBits::Glass | PhysicsMaterialTypeBits::Mirror,
    Group_Fabric  = PhysicsMaterialTypeBits::Fabric | PhysicsMaterialTypeBits::Carpet
                   | PhysicsMaterialTypeBits::Rope,
    Group_Paper   = PhysicsMaterialTypeBits::Paper | PhysicsMaterialTypeBits::Cardboard,
    Group_Plastic = PhysicsMaterialTypeBits::Plastic | PhysicsMaterialTypeBits::Rubber,
};

std::string to_string(PhysicsMaterialType type);

// Bitwise operators so PhysicsMaterialType can be combined/tested like any
// other flags enum (Group_* values are just PhysicsMaterialType values
// that happen to have more than one bit set).
constexpr PhysicsMaterialType operator|(PhysicsMaterialType a, PhysicsMaterialType b) noexcept
{
    return static_cast<PhysicsMaterialType>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}
constexpr PhysicsMaterialType operator&(PhysicsMaterialType a, PhysicsMaterialType b) noexcept
{
    return static_cast<PhysicsMaterialType>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}
constexpr PhysicsMaterialType operator^(PhysicsMaterialType a, PhysicsMaterialType b) noexcept
{
    return static_cast<PhysicsMaterialType>(static_cast<uint64_t>(a) ^ static_cast<uint64_t>(b));
}
constexpr PhysicsMaterialType operator~(PhysicsMaterialType a) noexcept
{
    return static_cast<PhysicsMaterialType>(~static_cast<uint64_t>(a));
}
constexpr PhysicsMaterialType & operator|=(PhysicsMaterialType & a, PhysicsMaterialType b) noexcept { return a = a | b; }
constexpr PhysicsMaterialType & operator&=(PhysicsMaterialType & a, PhysicsMaterialType b) noexcept { return a = a & b; }
constexpr PhysicsMaterialType & operator^=(PhysicsMaterialType & a, PhysicsMaterialType b) noexcept { return a = a ^ b; }

// ---------------------------------------------------------------------
// PhysicsMaterialHelper
//
// Stateless (from the caller's point of view) helper for classifying
// texture names into PhysicsMaterialType. Not instantiable - all
// functionality is exposed as static methods, backed by a
// lazily-constructed singleton that owns the compiled rule set.
// ---------------------------------------------------------------------
class PhysicsMaterialHelper
{
public:
    PhysicsMaterialHelper(const PhysicsMaterialHelper &) = delete;
    PhysicsMaterialHelper & operator=(const PhysicsMaterialHelper &) = delete;

    // Compiles every rule pattern. This happens automatically, lazily,
    // and thread-safely the first time Classify() is called, so calling
    // Initialize() is optional. Call it once at startup if you'd rather
    // pay the (small, one-time) compile cost up front instead of on the
    // first classification during gameplay.
    //
    // Returns true on success. Returns false if a rule pattern failed to
    // compile - this is a programmer error (bad hardcoded pattern
    // string), not a runtime/data condition, so callers can reasonably
    // assert(PhysicsMaterialHelper::Initialize()).
    static bool Initialize();

    // texturePath: engine-style texture identifier, WITHOUT extension, e.g.
    //     "GameData/textures/general/white"
    //     "textures/lq_conc/conc1_1"
    //     "conc1_1"                       (bare name also accepted)
    //
    // Only the last two path segments matter: the filename drives
    // matching, and its immediate parent folder is used as a fallback
    // when the filename alone doesn't contain a recognizable material
    // keyword (e.g. "aqsupp04" in textures/lq_tech/ falls back to
    // PhysicsMaterialType::Metal because that whole pack is sci-fi
    // metal/concrete paneling).
    //
    // Returns PhysicsMaterialType::Unknown if no rule and no folder
    // fallback apply - treat that as "needs a human to look at it", not
    // as an error. Always returns a single-bit leaf value (or Unknown),
    // never one of the Group_* combinations.
    static PhysicsMaterialType Classify(std::string_view texturePath);

    // True if `type` has at least one bit in common with `group`, e.g.
    //     IsInGroup(PhysicsMaterialType::MetalGrate, PhysicsMaterialType::Group_Metal) == true
    // `group` doesn't have to be one of the predefined Group_* values -
    // any bitwise-OR combination of PhysicsMaterialType works.
    static bool IsInGroup(PhysicsMaterialType type, PhysicsMaterialType group);

    // Debug/logging name. For a single-bit value (a leaf material, or
    // Group_Tile/Group_Wood/etc. that happen to be single-bit) returns
    // that material's name, e.g. "Metal". For a multi-bit value returns
    // its members joined with '|', e.g. "Blood|Flesh". Returns "Unknown"
    // for PhysicsMaterialType::Unknown and "INVALID" if no known bits
    // are set.
    static std::string ToString(PhysicsMaterialType type);

private:
    PhysicsMaterialHelper();
    ~PhysicsMaterialHelper();

    static PhysicsMaterialHelper & Instance();

    PhysicsMaterialType ClassifyImpl(std::string_view texturePath) const;

    struct Impl;
    Impl * m_impl; // owned; never null after construction succeeds
};
