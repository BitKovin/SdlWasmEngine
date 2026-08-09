// PhysicsMaterialHelper.cpp
//
// DESIGN NOTES (decisions made while building the original classifier,
// worth knowing if you extend the rule table below):
//
//  - Remimu has no built-in "search anywhere in string" API, only anchored
//    matching starting at a given index (see regex_match's start_i param).
//    RegexSearchCaseInsensitive() below emulates unanchored search by
//    retrying the match at every starting offset, left to right, and
//    returning the first (leftmost) hit. That's the standard trick for
//    backtracking engines without a native search entry point.
//
//  - Remimu has no case-insensitive flag, so all matching happens against
//    a lowercased copy of the input, and every rule pattern is written in
//    lowercase.
//
//  - Rules are tried in priority order, first match wins. Order matters:
//    organic keywords (blood/flesh/bone) are checked before generic
//    surface words (e.g. "tile") because "fleshtile" must classify as
//    Flesh, not Tile.
//
//  - Only well-known, unambiguous multi-letter map-type suffixes are
//    stripped before matching (_fbr "fullbright", _orm, _em, _normal...).
//    Bare single-letter suffixes (_a, _b, _m, _n, _s...) are deliberately
//    NOT stripped: in this asset set they are almost always tile-variant
//    letters (flat_01_a .. flat_01_p) rather than map-type markers, and
//    blindly stripping them would corrupt more names than it fixes.
//
//  - Two Quake-editor filename conventions show up throughout this
//    archive and are stripped as noise before matching (they don't carry
//    material information, just metadata):
//      "+0" / "+1" .. "+9" / "+a" .. "+j"   -- animated-texture frame index
//      "plus_0_" / "plus_a_" etc.           -- the same thing, spelled out
//      leading "{"                          -- color-key transparency marker
//    ("star_water1" etc. is the same idea for the classic Quake "*water"
//    liquid-surface convention, spelled out as "star_" -- no stripping
//    needed there since "water"/"lava"/"slime" still matches as a
//    substring either way.)
//
//  - Folder-name fallback (used only when no filename rule matches) was
//    validated against a real 2,920-file archive. Two folders turned out
//    to not be "surface materials" at all: lq_palette is flat color/shape
//    swatches (falls back to Dev), and lq_health_ammo plus most of
//    psx_mega_pack / psx_textures_2 are individual prop-item skins
//    (battery, radio, cassette...) that don't reduce to one physical
//    material by name alone -- they fall back to PropMisc rather than
//    being force-fit into a surface category.
//
//  - Classify() always returns a single-bit leaf value (or Unknown),
//    never one of the Group_* combinations - grouping is purely a
//    post-classification lookup for gameplay code (see IsInGroup()).

#include "PhysicsMaterialHelper.h"

#include "remimu.h"

#include <array>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    constexpr size_t kMaxTokensPerRule = 64;
    // The suffix-strip pattern is one big alternation (22 words), which in
    // Remimu's per-character token model needs far more token slots than
    // the small single-keyword rule patterns above.
    constexpr size_t kMaxTokensSuffix = 512;

    struct RuleSpec
    {
        const char * pattern;
        PhysicsMaterialType material;
    };

    // Priority-ordered: first match wins. See file header for ordering notes.
    constexpr RuleSpec kRules[] =
    {
        // --- Non-rendering / engine-only (checked first: never let these
        //     accidentally match a visual-material keyword) -----------------
        { "trigger",                 PhysicsMaterialType::Utility },
        { "\\bclip\\b",              PhysicsMaterialType::Utility },
        { "hintskip",                PhysicsMaterialType::Utility },
        { "\\bhint\\b",              PhysicsMaterialType::Utility },
        { "\\bskip\\b",              PhysicsMaterialType::Utility },
        { "\\borigin\\b",            PhysicsMaterialType::Utility },
        { "nodraw",                  PhysicsMaterialType::Utility },
        { "__tb_empty",              PhysicsMaterialType::Utility },

        // --- Sky -------------------------------------------------------------
        { "sky",                     PhysicsMaterialType::Sky },

        // --- Organic (must precede generic surface words, e.g. "fleshtile") --
        { "blood",                   PhysicsMaterialType::Blood },
        { "flesh",                   PhysicsMaterialType::Flesh },
        { "\\bmeat\\b",              PhysicsMaterialType::Flesh },
        { "\\bgut",                  PhysicsMaterialType::Flesh },
        { "\\bskin\\b",              PhysicsMaterialType::Flesh },
        { "bone",                    PhysicsMaterialType::Bone },
        { "skull",                   PhysicsMaterialType::Bone },
        { "spine",                   PhysicsMaterialType::Bone },

        // --- Liquids -----------------------------------------------------------
        { "lava",                    PhysicsMaterialType::Lava },
        { "slime",                   PhysicsMaterialType::Slime },
        { "\\bacid\\b",              PhysicsMaterialType::Slime },
        { "water",                   PhysicsMaterialType::Water },
        { "wfall",                   PhysicsMaterialType::Water },

        // --- Metal (grate/mesh is more specific than plain metal: check first)
        { "grat(e|ing)",             PhysicsMaterialType::MetalGrate },
        { "\\bmesh\\b",              PhysicsMaterialType::MetalGrate },
        { "chainlink",               PhysicsMaterialType::MetalGrate },
        { "metal",                   PhysicsMaterialType::Metal },
        { "\\bmetl",                 PhysicsMaterialType::Metal },
        { "\\bmet_",                 PhysicsMaterialType::Metal },
        { "\\bgig[0-9]",             PhysicsMaterialType::Metal },
        { "\\brust",                 PhysicsMaterialType::Metal },
        { "steel",                   PhysicsMaterialType::Metal },
        { "corrugated",              PhysicsMaterialType::Metal },
        { "\\brivet",                PhysicsMaterialType::Metal },
        { "\\briv(g|s)\\b",          PhysicsMaterialType::Metal },

        // --- Wood ----------------------------------------------------------------
        { "wood",                    PhysicsMaterialType::Wood },
        { "\\bplank",                PhysicsMaterialType::Wood },
        { "\\btimber",               PhysicsMaterialType::Wood },
        { "\\bcrate",                PhysicsMaterialType::Wood },

        // --- Cardboard / paper (before generic checks) --------------------------
        { "cardboard",               PhysicsMaterialType::Cardboard },
        { "\\bpaper",                PhysicsMaterialType::Paper },

        // --- Stone family ----------------------------------------------------
        { "stone",                   PhysicsMaterialType::Stone },
        { "cobble",                  PhysicsMaterialType::Stone },
        { "cobstn",                  PhysicsMaterialType::Stone },
        { "\\brock",                 PhysicsMaterialType::Stone },
        { "\\brck\\b",               PhysicsMaterialType::Stone },
        { "darkrock",                PhysicsMaterialType::Stone },
        { "marb",                    PhysicsMaterialType::Marble },

        // --- Brick / masonry -------------------------------------------------------
        { "brick",                   PhysicsMaterialType::Brick },
        { "\\bbrk\\b",               PhysicsMaterialType::Brick },
        { "ebrick",                  PhysicsMaterialType::Brick },
        { "dbrick",                  PhysicsMaterialType::Brick },

        // --- Concrete ------------------------------------------------------------
        { "concrete",                PhysicsMaterialType::Concrete },
        { "\\bconc",                 PhysicsMaterialType::Concrete },

        // --- Ground / terrain ----------------------------------------------------
        { "plaster",                 PhysicsMaterialType::Plaster },
        { "gravel",                  PhysicsMaterialType::Gravel },
        { "asphalt",                 PhysicsMaterialType::Asphalt },
        { "\\bdirt",                 PhysicsMaterialType::Dirt },
        { "\\bdrt\\b",               PhysicsMaterialType::Dirt },
        { "\\bmud\\b",               PhysicsMaterialType::Dirt },
        { "grass",                   PhysicsMaterialType::Grass },
        { "lawn",                    PhysicsMaterialType::Grass },
        { "\\bsand\\b",              PhysicsMaterialType::Sand },
        { "\\bsnow",                 PhysicsMaterialType::Snow },
        { "\\bleaf",                 PhysicsMaterialType::Foliage },
        { "\\bleaves\\b",            PhysicsMaterialType::Foliage },
        { "\\bvine",                 PhysicsMaterialType::Foliage },

        // --- Tile / linoleum (after organic + stone/brick, see header note) ------
        { "linoleum",                PhysicsMaterialType::Tile },
        { "\\btile",                 PhysicsMaterialType::Tile },

        // --- Glass -----------------------------------------------------------------
        { "mirror",                  PhysicsMaterialType::Mirror },
        { "glass",                   PhysicsMaterialType::Glass },
        { "\\bwindow",               PhysicsMaterialType::Glass },

        // --- Fabric family -----------------------------------------------------
        { "tapestry",                PhysicsMaterialType::Fabric },
        { "fabric",                  PhysicsMaterialType::Fabric },
        { "\\bcurtain",              PhysicsMaterialType::Fabric },
        { "\\bbanner",               PhysicsMaterialType::Fabric },
        { "carpet",                  PhysicsMaterialType::Carpet },

        // --- Plastic / rubber --------------------------------------------------
        { "plastic",                 PhysicsMaterialType::Plastic },
        { "\\brubber",               PhysicsMaterialType::Rubber },

        // --- Rope / cable / wire -------------------------------------------------
        { "\\brope\\b",              PhysicsMaterialType::Rope },
        { "\\bcable",                PhysicsMaterialType::Rope },
        { "\\bwire",                 PhysicsMaterialType::Rope },

        // --- Dev / placeholder (low priority, before folder fallback) ------------
        { "^dot_",                   PhysicsMaterialType::Dev },
        { "\\bdev\\b",               PhysicsMaterialType::Dev },
        { "\\bcheck(er)?\\b",        PhysicsMaterialType::Dev },
        { "\\bmissing\\b",           PhysicsMaterialType::Dev },
        { "\\bnull\\b",              PhysicsMaterialType::Dev },
        { "\\bplaceholder\\b",       PhysicsMaterialType::Dev },
    };
    constexpr size_t kRuleCount = sizeof(kRules) / sizeof(kRules[0]);

    // Trailing map-type suffix tokens to strip before classification (explicit,
    // unambiguous multi-letter tokens only -- see file header note).
    constexpr const char * kSuffixStripPattern =
        "_(fbr|orm|em|ao|normal|norm|rough|roughness|spec|specular|diff|diffuse"
        "|albedo|alb|mask|luma|glow|disp|displacement|height|bump|nrm)$";

    // Leading Quake-style animation-frame index, e.g. "+0", "+a".
    constexpr const char * kAnimPrefixPattern = "^\\+[0-9a-j]";

    // The same convention spelled out, e.g. "plus_0_", "plus_a_".
    constexpr const char * kAnimPrefixWordyPattern = "^plus_[0-9a-j]_?";

    struct FolderFallback
    {
        const char * folder; // lowercase
        PhysicsMaterialType material;
    };

    // Only consulted when no filename rule matched. Folders not listed here
    // (or listed with PhysicsMaterialType::Unknown) intentionally fall
    // through to Unknown so genuinely unclassifiable content (a noise
    // texture, a particle "smoke"/"trail" sprite) surfaces for manual
    // review instead of being force-fit into a material.
    constexpr FolderFallback kFolderFallbacks[] =
    {
        { "ground",          PhysicsMaterialType::Grass },
        { "building_1",      PhysicsMaterialType::Brick },
        { "common",          PhysicsMaterialType::Utility },
        { "delvenpack",      PhysicsMaterialType::Stone },
        { "generic",         PhysicsMaterialType::Dev },
        { "lq_conc",         PhysicsMaterialType::Concrete },
        { "lq_dev",          PhysicsMaterialType::Dev },
        { "lq_flesh",        PhysicsMaterialType::Flesh },
        { "lq_greek",        PhysicsMaterialType::Stone },
        { "lq_health_ammo",  PhysicsMaterialType::PropMisc },
        { "lq_legacy",       PhysicsMaterialType::Stone },
        { "lq_liquidsky",    PhysicsMaterialType::Water },
        { "lq_mayan",        PhysicsMaterialType::Stone },
        { "lq_medieval",     PhysicsMaterialType::Stone },
        { "lq_metal",        PhysicsMaterialType::Metal },
        { "lq_palette",      PhysicsMaterialType::Dev },
        { "lq_props",        PhysicsMaterialType::Wood },
        { "lq_tech",         PhysicsMaterialType::Metal },
        { "lq_terra",        PhysicsMaterialType::Dirt },
        { "lq_utility",      PhysicsMaterialType::Utility },
        { "lq_wood",         PhysicsMaterialType::Wood },
        { "metal",           PhysicsMaterialType::Metal },
        { "psx_mega_pack",   PhysicsMaterialType::PropMisc },
        { "psx_textures_2",  PhysicsMaterialType::PropMisc },
        { "skies",           PhysicsMaterialType::Sky },
        { "tormentpack",     PhysicsMaterialType::Stone },
        { "wall",            PhysicsMaterialType::Brick },
        { "water",           PhysicsMaterialType::Water },
        { "wood",            PhysicsMaterialType::Wood },
        // "noise" and "particles" deliberately absent: their remaining
        // unmatched members (grainy noise, smoke/trail sprites) aren't a
        // physical surface material at all.
    };
    constexpr size_t kFolderFallbackCount = sizeof(kFolderFallbacks) / sizeof(kFolderFallbacks[0]);

    // Leaf material name, indexed by bit position (see PhysicsMaterialTypeBits
    // in the header) - used by ToString() to turn a single-bit value, or a
    // multi-bit Group_* value, into a readable name.
    constexpr std::array<const char *, PhysicsMaterialTypeBits::LeafCount> kLeafNames =
    {
        "Utility", "Dev", "Sky", "PropMisc",
        "Stone", "Rock", "Brick", "Concrete", "Marble", "Tile", "Plaster",
        "Metal", "MetalGrate",
        "Wood",
        "Dirt", "Grass", "Sand", "Gravel", "Asphalt", "Snow", "Ice", "Foliage",
        "Water", "Lava", "Slime",
        "Blood", "Flesh", "Bone",
        "Glass", "Mirror", "Fabric", "Carpet", "Paper", "Cardboard", "Plastic", "Rubber", "Rope",
    };

    // ---------------------------------------------------------------------
    // Remimu doesn't provide a "search anywhere" API (only anchored matching
    // at a caller-supplied start_i) -- see file header. This retries the
    // match at each offset and returns the leftmost hit, which is exactly
    // what a regex_search()/"contains" call would do.
    // ---------------------------------------------------------------------
    int64_t RegexSearchCaseInsensitive(const RegexToken * tokens, const std::string & text)
    {
        const size_t len = text.size();
        for (size_t startIndex = 0; startIndex <= len; startIndex++)
        {
            const int64_t m = regex_match(tokens, text.c_str(), startIndex, 0, nullptr, nullptr);
            if (m >= 0)
                return static_cast<int64_t>(startIndex);
        }
        return -1;
    }

    void LowercaseInPlace(std::string & s)
    {
        for (char & c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // Splits "GameData/textures/general/white" into name="white" and
    // folder="general" (the segment immediately above it). Either output
    // can be an empty string if the path is too short to contain it.
    void SplitPath(std::string_view texturePath, std::string & outName, std::string & outFolder)
    {
        const size_t lastSlash = texturePath.find_last_of('/');

        const std::string_view nameView = (lastSlash != std::string_view::npos)
            ? texturePath.substr(lastSlash + 1)
            : texturePath;
        outName.assign(nameView);

        outFolder.clear();
        if (lastSlash != std::string_view::npos && lastSlash > 0)
        {
            const std::string_view beforeName = texturePath.substr(0, lastSlash);
            const size_t prevSlash = beforeName.find_last_of('/');
            const std::string_view folderView = (prevSlash != std::string_view::npos)
                ? beforeName.substr(prevSlash + 1)
                : beforeName;
            outFolder.assign(folderView);
        }
    }

} // namespace

// ---------------------------------------------------------------------
// PhysicsMaterialHelper::Impl
//
// Owns the compiled regex rule set. Constructed once, lazily, behind
// PhysicsMaterialHelper::Instance().
// ---------------------------------------------------------------------
struct PhysicsMaterialHelper::Impl
{
    struct CompiledRule
    {
        std::array<RegexToken, kMaxTokensPerRule> tokens{};
        int16_t tokenCount = 0;
        PhysicsMaterialType material = PhysicsMaterialType::Unknown;
    };

    std::array<CompiledRule, kRuleCount> rules;

    std::array<RegexToken, kMaxTokensSuffix> suffixTokens{};
    int16_t suffixTokenCount = 0;

    std::array<RegexToken, kMaxTokensPerRule> animPrefixTokens{};
    int16_t animPrefixTokenCount = 0;

    std::array<RegexToken, kMaxTokensPerRule> animPrefixWordyTokens{};
    int16_t animPrefixWordyTokenCount = 0;

    bool initSucceeded = false;

    Impl()
    {
        initSucceeded = true;

        for (size_t i = 0; i < kRuleCount; i++)
        {
            int16_t tokenCount = kMaxTokensPerRule;
            const int err = regex_parse(kRules[i].pattern, rules[i].tokens.data(), &tokenCount, 0);
            if (err != 0)
            {
                std::fprintf(stderr,
                              "PhysicsMaterialHelper: rule %zu (\"%s\") failed to compile (err=%d)\n",
                              i, kRules[i].pattern, err);
                initSucceeded = false;
                continue;
            }
            rules[i].tokenCount = tokenCount;
            rules[i].material = kRules[i].material;
        }

        suffixTokenCount = kMaxTokensSuffix;
        if (regex_parse(kSuffixStripPattern, suffixTokens.data(), &suffixTokenCount, 0) != 0)
        {
            std::fprintf(stderr, "PhysicsMaterialHelper: suffix-strip pattern failed to compile\n");
            initSucceeded = false;
        }

        animPrefixTokenCount = kMaxTokensPerRule;
        if (regex_parse(kAnimPrefixPattern, animPrefixTokens.data(), &animPrefixTokenCount, 0) != 0)
        {
            std::fprintf(stderr, "PhysicsMaterialHelper: anim-prefix pattern failed to compile\n");
            initSucceeded = false;
        }

        animPrefixWordyTokenCount = kMaxTokensPerRule;
        if (regex_parse(kAnimPrefixWordyPattern, animPrefixWordyTokens.data(), &animPrefixWordyTokenCount, 0) != 0)
        {
            std::fprintf(stderr, "PhysicsMaterialHelper: anim-prefix-wordy pattern failed to compile\n");
            initSucceeded = false;
        }
    }

    // Strips a leading '{' (Quake color-key transparency marker) and a
    // leading animated-frame index ("+0".."+9", "+a".."+j", or the
    // spelled-out "plus_0_" form). Operates on `name` in place by erasing
    // the stripped prefix.
    void StripPrefixNoise(std::string & name) const
    {
        size_t offset = 0;
        if (!name.empty() && name[0] == '{')
            offset = 1;

        const char * buf = name.c_str() + offset;

        int64_t m = regex_match(animPrefixTokens.data(), buf, 0, 0, nullptr, nullptr);
        if (m >= 0)
        {
            offset += static_cast<size_t>(m); // "^\+[0-9a-j]" always matches at start_i 0 when it matches
            name.erase(0, offset);
            return;
        }

        m = regex_match(animPrefixWordyTokens.data(), buf, 0, 0, nullptr, nullptr);
        if (m >= 0)
        {
            offset += static_cast<size_t>(m);
            name.erase(0, offset);
            return;
        }

        name.erase(0, offset);
    }

    // Strips a trailing map-type suffix (see kSuffixStripPattern), in
    // place, up to a few times in case of a doubled-up suffix.
    void StripSuffixNoise(std::string & name) const
    {
        for (int pass = 0; pass < 3; pass++)
        {
            const int64_t foundAt = RegexSearchCaseInsensitive(suffixTokens.data(), name);
            if (foundAt < 0)
                break;
            name.resize(static_cast<size_t>(foundAt));
        }
    }

    PhysicsMaterialType FolderFallback(const std::string & folderLower) const
    {
        for (size_t i = 0; i < kFolderFallbackCount; i++)
        {
            if (folderLower == kFolderFallbacks[i].folder)
                return kFolderFallbacks[i].material;
        }
        return PhysicsMaterialType::Unknown;
    }

    PhysicsMaterialType Classify(std::string_view texturePath) const
    {
        std::string name;
        std::string folder;
        SplitPath(texturePath, name, folder);

        LowercaseInPlace(name);
        LowercaseInPlace(folder);

        StripPrefixNoise(name);
        StripSuffixNoise(name);

        for (size_t i = 0; i < kRuleCount; i++)
        {
            if (RegexSearchCaseInsensitive(rules[i].tokens.data(), name) >= 0)
                return rules[i].material;
        }

        return FolderFallback(folder);
    }
};

PhysicsMaterialHelper::PhysicsMaterialHelper()
    : m_impl(new Impl())
{
}

PhysicsMaterialHelper::~PhysicsMaterialHelper()
{
    delete m_impl;
}

PhysicsMaterialHelper & PhysicsMaterialHelper::Instance()
{
    // Function-local static: constructed on first use, thread-safe
    // (guaranteed by the standard since C++11), destroyed at program exit.
    static PhysicsMaterialHelper instance;
    return instance;
}

bool PhysicsMaterialHelper::Initialize()
{
    return Instance().m_impl->initSucceeded;
}

PhysicsMaterialType PhysicsMaterialHelper::ClassifyImpl(std::string_view texturePath) const
{
    return m_impl->Classify(texturePath);
}

PhysicsMaterialType PhysicsMaterialHelper::Classify(std::string_view texturePath)
{
    return Instance().ClassifyImpl(texturePath);
}

bool PhysicsMaterialHelper::IsInGroup(PhysicsMaterialType type, PhysicsMaterialType group)
{
    return (type & group) != PhysicsMaterialType::Unknown;
}

std::string PhysicsMaterialHelper::ToString(PhysicsMaterialType type)
{
    const uint64_t bits = static_cast<uint64_t>(type);
    if (bits == 0)
        return "Unknown";

    std::string result;
    for (size_t i = 0; i < kLeafNames.size(); i++)
    {
        if (bits & (1ULL << i))
        {
            if (!result.empty())
                result += '|';
            result += kLeafNames[i];
        }
    }
    return result.empty() ? "INVALID" : result;
}

std::string to_string(PhysicsMaterialType type)
{
    if (type == PhysicsMaterialType::Unknown) return "Unknown";

    std::string result;
    uint64_t val = static_cast<uint64_t>(type);

    auto check_and_add = [&](PhysicsMaterialType flag, const char* name) {
        uint64_t flag_val = static_cast<uint64_t>(flag);
        if ((val & flag_val) == flag_val) {
            if (!result.empty()) result += " | ";
            result += name;
            val &= ~flag_val; // Clear the bit so we don't count it twice
        }
        };

    // Check unique groups first so they don't break down into individual parts
    check_and_add(PhysicsMaterialType::Group_Stone, "Group_Stone");
    check_and_add(PhysicsMaterialType::Group_Metal, "Group_Metal");
    check_and_add(PhysicsMaterialType::Group_Grass, "Group_Grass");
    check_and_add(PhysicsMaterialType::Group_Flesh, "Group_Flesh");
    check_and_add(PhysicsMaterialType::Group_Glass, "Group_Glass");
    check_and_add(PhysicsMaterialType::Group_Fabric, "Group_Fabric");
    check_and_add(PhysicsMaterialType::Group_Paper, "Group_Paper");
    check_and_add(PhysicsMaterialType::Group_Plastic, "Group_Plastic");

    // Check base types
    check_and_add(PhysicsMaterialType::Utility, "Utility");
    check_and_add(PhysicsMaterialType::Dev, "Dev");
    check_and_add(PhysicsMaterialType::Sky, "Sky");
    check_and_add(PhysicsMaterialType::PropMisc, "PropMisc");
    check_and_add(PhysicsMaterialType::Stone, "Stone");
    check_and_add(PhysicsMaterialType::Rock, "Rock");
    check_and_add(PhysicsMaterialType::Brick, "Brick");
    check_and_add(PhysicsMaterialType::Concrete, "Concrete");
    check_and_add(PhysicsMaterialType::Marble, "Marble");
    check_and_add(PhysicsMaterialType::Tile, "Tile");
    check_and_add(PhysicsMaterialType::Plaster, "Plaster");
    check_and_add(PhysicsMaterialType::Metal, "Metal");
    check_and_add(PhysicsMaterialType::MetalGrate, "MetalGrate");
    check_and_add(PhysicsMaterialType::Wood, "Wood");
    check_and_add(PhysicsMaterialType::Dirt, "Dirt");
    check_and_add(PhysicsMaterialType::Grass, "Grass");
    check_and_add(PhysicsMaterialType::Sand, "Sand");
    check_and_add(PhysicsMaterialType::Gravel, "Gravel");
    check_and_add(PhysicsMaterialType::Asphalt, "Asphalt");
    check_and_add(PhysicsMaterialType::Snow, "Snow");
    check_and_add(PhysicsMaterialType::Ice, "Ice");
    check_and_add(PhysicsMaterialType::Foliage, "Foliage");
    check_and_add(PhysicsMaterialType::Water, "Water");
    check_and_add(PhysicsMaterialType::Lava, "Lava");
    check_and_add(PhysicsMaterialType::Slime, "Slime");
    check_and_add(PhysicsMaterialType::Blood, "Blood");
    check_and_add(PhysicsMaterialType::Flesh, "Flesh");
    check_and_add(PhysicsMaterialType::Bone, "Bone");
    check_and_add(PhysicsMaterialType::Glass, "Glass");
    check_and_add(PhysicsMaterialType::Mirror, "Mirror");
    check_and_add(PhysicsMaterialType::Fabric, "Fabric");
    check_and_add(PhysicsMaterialType::Carpet, "Carpet");
    check_and_add(PhysicsMaterialType::Paper, "Paper");
    check_and_add(PhysicsMaterialType::Cardboard, "Cardboard");
    check_and_add(PhysicsMaterialType::Plastic, "Plastic");
    check_and_add(PhysicsMaterialType::Rubber, "Rubber");
    check_and_add(PhysicsMaterialType::Rope, "Rope");

    return result.empty() ? "Unknown/Unhandled" : result;
}
