#pragma once

#include <glm.h>

// ─────────────────────────────────────────────────────────────────────────────
// IMPORTANT: this header must never include vaudio.h, and must never expose a
// vaudio type (VAEmitter, VALowPassFilter, VAEAXReverb, VAMaterialType, ...)
// in its public interface. vaudio.h is included in exactly one place in the
// whole engine: SpatialSoundManager.cpp. That's deliberate — using vaudio
// requires a license, and keeping every vaudio symbol confined to that one
// translation unit means the entire spatial audio system (and its licensing
// dependency) can be dropped from the project by deleting/stubbing that one
// .cpp file, without touching this header or any of its callers
// (SoundManager, SoundInstance, FmodEventInstance, ...).
//
// Everything below is a plain, engine-owned type. SpatialSoundManager.cpp is
// responsible for converting to/from the real vaudio types internally.
// ─────────────────────────────────────────────────────────────────────────────

// Opaque handle to one live positional sound's spatial-audio state (in
// practice a vaudio emitter, but callers never know that — the pointer is
// only ever dereferenced inside SpatialSoundManager.cpp). Never construct or
// dereference this yourself; only pass it back to the functions below.
struct SpatialSoundEmitter;

// Occlusion/permeation/ambient low-pass filter: two multiplicative gain terms
// (0 = fully filtered, 1 = unfiltered), one for low frequencies and one for
// high. Apply directly to any lowpass filter/EQ (OpenAL AL_FILTER_LOWPASS,
// an FMOD EQ DSP, etc).
struct SpatialLowPassFilter
{
    float gainLF = 1.0f;
    float gainHF = 1.0f;
};

// Plain, engine-owned reverb description — a stable subset of vaudio's EAX
// reverb result, expressed as parameters any reverb effect/DSP can consume
// (OpenAL EFX EAXReverb, FMOD SFXREVERB, a custom DSP, etc), without exposing
// vaudio's own struct outside SpatialSoundManager.cpp. Field meanings match
// the EAX Reverb parameter set.
struct SpatialReverb
{
    float density = 1.0f;
    float diffusion = 1.0f;
    float gain = 0.32f;
    float gainHF = 0.89f;
    float gainLF = 1.0f;
    float decayTime = 1.49f;
    float decayHFRatio = 0.83f;
    float decayLFRatio = 1.0f;
    float reflectionsGain = 0.05f;
    float reflectionsDelay = 0.007f;
    float lateReverbGain = 1.26f;
    float lateReverbDelay = 0.011f;
    float echoTime = 0.25f;
    float echoDepth = 0.0f;
    float modulationTime = 0.25f;
    float modulationDepth = 0.0f;
    float airAbsorptionGainHF = 0.994f;
    float hfReference = 5000.0f;
    float lfReference = 250.0f;
    float roomRolloffFactor = 0.0f;
    bool  decayHFLimit = true;

    // Pan vector for the reflections/late-reverb portion of this reverb,
    // relative to the listener — NOT normalized. Magnitude (0..1) is how
    // directional the reflected energy is (0 = fully diffuse, surrounds the
    // listener; 1 = arriving strongly from one direction), independent of
    // 'gain' above. Maps directly onto OpenAL's AL_EAXREVERB_REFLECTIONS_PAN
    // / AL_EAXREVERB_LATE_REVERB_PAN. Zero vector (the default) means "no
    // directional data yet" — always safe to apply as-is.
    //
    // Derived from vaudio's Grouped EAX system: every currently-playing
    // positional sound's own emitter also casts a light set of reverb rays
    // and contributes to the world's pool of "rooms" (see
    // SpatialSoundManager::CreateSoundEmitter and BuildWorld's
    // vaEmitterSetHasRelativeReverb). This vector is the gain-weighted sum
    // of each room's direction relative to the listener, so it naturally
    // favors whichever nearby rooms currently have sound playing in them —
    // a room with no active sound source in it won't contribute a distinct
    // directional cue until something starts playing there.
    glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
};

// ─────────────────────────────────────────────────────────────────────────────
// SpatialSoundManager
//
// Builds and maintains a vaudio world from the currently loaded BSP level,
// and exposes a small query API that SoundManager (the only other file that
// touches this system) uses to layer realistic occlusion + reverb on top of
// whatever effects a sound designer has already authored. SoundInstance and
// FmodEventInstance never call this class directly (see SoundManager instead)
// — see the note above about why.
//
// Design notes:
//  - This system is entirely self-contained and passive. It reads BSP/Level/
//    Entity data that already exists (the same way Physics builds its
//    collision scene) and never modifies or hooks into Entity/Level/BSP code.
//  - World geometry ownership follows entity ownership: every mesh primitive
//    built from a brush model is tracked against the owning entity's Id. Each
//    frame we check whether that entity still exists (Level::FindEntityWithId)
//    and destroy the primitives the moment it doesn't, instead of relying on
//    a destruction callback/hook.
//  - A brush entity's movement (dynamic doors, platforms, etc) is reflected by
//    updating the primitive's transform from Entity::Position/Rotation/Scale
//    every frame, unless Entity::Static is true, in which case the transform
//    is set once at build time and never touched again.
// ─────────────────────────────────────────────────────────────────────────────
class SpatialSoundManager
{
public:

    // Called once at the end of Level::OpenLevel(), after navigation
    // compilation. (Re)builds the vaudio world from the freshly loaded BSP
    // level and every entity currently in the level.
    static void BuildWorld();

    // Called every frame (from SoundManager::Update()). Advances the vaudio
    // raytracing simulation, moves the listener to the camera, updates the
    // transforms of non-static brush geometry, and prunes audio geometry for
    // any entity that has been destroyed since the last tick.
    static void Update();

    // Tears down the vaudio world and every primitive/emitter it owns.
    // Safe to call even if no world has been built. BuildWorld() calls this
    // internally before rebuilding, so most callers won't need this — it's
    // exposed for full engine shutdown.
    static void Shutdown();

    // Debug: draws the wireframe of every triangle currently fed to vaudio
    // (i.e. exactly what BuildWorld()/Update() built and is being raytraced
    // against — the same brush geometry, in world space, transform included)
    // via DebugDraw::Line. Off by default; enable from a debug menu/console
    // command. Drawn once per Update(), so toggling this on for a large level
    // means one DebugDraw::Line call per triangle edge, every frame — expect
    // it to be noticeably heavier than normal play.
    static void SetDebugDrawGeometry(bool enabled);

    // ── Positional sound-emitter API ─────────────────────────────────────────
    // Represents one currently-playing positional sound. SoundManager creates
    // one when such a sound starts playing and releases it when the sound
    // stops. Sounds with no position of their own (SoundInstanceBase::
    // EnvironmentalSound) never use these — see GetAmbientFilter()/
    // GetOutsidePercent() below instead.

    // Creates a new emitter at 'position' and registers it as a raytrace
    // target of the listener. Returns nullptr if no world exists yet (e.g.
    // called before the first BuildWorld(), or DISABLE_VAUDIO is set).
    static SpatialSoundEmitter* CreateSoundEmitter(const glm::vec3& position);

    // Unregisters and destroys a previously created sound emitter. Safe to
    // call with nullptr.
    static void ReleaseSoundEmitter(SpatialSoundEmitter* emitter);

    // Updates the world-space position of a previously created sound emitter.
    static void UpdateSoundEmitterPosition(SpatialSoundEmitter* emitter, const glm::vec3& position);

    // True once the listener has raytraced this emitter at least once, i.e.
    // GetOcclusionFilter() below is returning real (not neutral) data.
    static bool HasResultsFor(SpatialSoundEmitter* emitter);

    // Combined occlusion/permeation low-pass filter between the listener and
    // this emitter. Returns a neutral (1.0, 1.0) filter if there's no data
    // yet (or DISABLE_VAUDIO is set), so it's always safe to apply directly.
    static SpatialLowPassFilter GetOcclusionFilter(SpatialSoundEmitter* emitter);

    // ── "Environment sound" feature ──────────────────────────────────────────
    // For ambience/room-tone beds with no position of their own
    // (SoundInstanceBase::EnvironmentalSound). Driven by the listener's
    // *ambient* occlusion/permeation rays, which — unlike the regular
    // per-target occlusion rays above — are allowed to escape the level
    // bounds entirely. How much of that energy escapes is what
    // GetOutsidePercent() reports, and it's what makes this suitable for
    // "how open/enclosed does this space sound" rather than "is there a
    // wall between two specific points".

    // Ambient low-pass filter — apply directly to an EnvironmentalSound
    // instance's own filter, the same way GetOcclusionFilter() is applied to
    // a positional sound's.
    static SpatialLowPassFilter GetAmbientFilter();

    // 0..1 — the fraction of the listener's ambient ray energy that escaped
    // the level bounds (0 = fully enclosed, 1 = wide open / outdoors).
    // Available for driving things like an outdoor-ambience bed's volume;
    // not applied automatically by anything in this system.
    static float GetOutsidePercent();

    // ── Listener reverb ───────────────────────────────────────────────────────
    // True once vaudio has produced at least one reverb result this level.
    static bool IsReverbReady();

    // The listener's own EAX reverb — the ambience of the space the listener
    // is currently in, blended with directional data from nearby rooms (see
    // SpatialReverb::direction above). Writes into 'outReverb' and returns
    // true if data is available (i.e. IsReverbReady()); leaves 'outReverb'
    // untouched and returns false otherwise.
    static bool GetListenerReverb(SpatialReverb& outReverb);
};