#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include "fmod_include.h"
#include <SDL2/SDL_audio.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "SoundBufferData.h"
#include "../Camera.h"
#include "../glm.h"
#include "SoundInstance.hpp"

class SoundManager
{
public:
    // ── OpenAL contexts ──────────────────────────────────────────────────────
    /// HRTF-enabled context — owns all mono (3D spatial) sources.
    static ALCcontext* contextMono;
    /// Flat-stereo context — owns all stereo / 2D / UI sources.
    /// A separate context is needed because HRTF applied to a pre-mixed stereo
    /// stream produces distorted, incorrect output.
    static ALCcontext* contextStereo;

    // ── Shared audio mutex ───────────────────────────────────────────────────
    /// Must be held for every OpenAL call and every access to the source pool.
    /// Declared recursive so nested calls (e.g. Stop → Release → Pool::Release)
    /// don't deadlock.
    inline static std::recursive_mutex audioMutex;

    // ── FMOD systems ─────────────────────────────────────────────────────────
    inline static FMOD::Studio::System* studioSystem = nullptr;
    inline static FMOD::System*         coreSystem   = nullptr;

    // ── Volume ───────────────────────────────────────────────────────────────
    static float GlobalVolume;
    static float SfxVolume;
    static float MusicVolume;
	static float VoiceVolume;
	static float UiVolume;

    // ── Lifecycle ────────────────────────────────────────────────────────────
    static void Initialize();
    static void Update();
    static void Close();

    // ── Asset management ─────────────────────────────────────────────────────
    static SoundBufferData                    LoadOrGetSoundFileBuffer(const std::string& path);
    static std::shared_ptr<SoundInstance>     GetSoundFromPath(const std::string& path);
    static void                               CleanAllData();

    // ── FMOD bank loading ─────────────────────────────────────────────────────
    static FMOD::Studio::Bank* LoadBankFromPath(const std::string& bankPath,
                                                 bool loadSampleData = true);

    static float GetVolumeForSoundType(SoundType type);

    // ─────────────────────────────────────────────────────────────────────────
    // Spatial audio (vaudio) integration.
    //
    // This — together with SpatialSound/SpatialSoundManager.cpp — is the ONLY
    // place in the engine that's aware of the spatial audio backend.
    // SoundInstance and FmodEventInstance never reference it directly; they
    // only ever call the plain functions below with their own native handles.
    // This keeps the licensed vaudio dependency confined to two files instead
    // of leaking into every sound-playing class in the engine.
    //
    // All effects here are per sound instance — every SoundInstance/
    // FmodEventInstance gets its own independent filter/DSP state, never a
    // shared master-bus effect, so simultaneous sounds never fight over (or
    // smear together) each other's occlusion/reverb.
    // ─────────────────────────────────────────────────────────────────────────

    // Called every frame a spatial-audio-eligible OpenAL sound is updating
    // (see SoundInstance::UpdateSourceParams()). Creates/updates this sound's
    // internal spatial state (keyed by `key` — pass the SoundInstance's own
    // `this`) and writes fresh occlusion/ambient + environment reverb values
    // directly onto the AL objects the caller already owns and passes in.
    //
    //   key                - stable per-instance identity for this sound's whole lifetime
    //   source             - the AL source this sound is currently playing on
    //   filter             - an AL_FILTER_LOWPASS object the caller owns; combined occlusion/ambient
    //                        gain (+ the caller's own artist filter, if enabled) is written into it
    //                        and bound to AL_DIRECT_FILTER
    //   envReverbEffect/envReverbSlot - effect + aux slot objects the caller owns; filled from the
    //                        listener's reverb and routed to auxiliary send slot 2 (kept clear of the
    //                        artist-authored EnableEcho [slot 0] / EnableReverb [slot 1])
    //   position           - world-space position (ignored when `environmental` is true)
    //   environmental      - see SoundInstanceBase::EnvironmentalSound
    //   disableSpatial     - see SoundInstanceBase::DisableSpatial
    //   isUISound          - see SoundInstanceBase::IsUISound
    //   artistFilterEnabled/artistLowPassGain/artistLowPassGainHF - the caller's own
    //                        EnableFilter/LowPassGain/LowPassGainHF, multiplied into the occlusion
    //                        result (ignored for `environmental` sounds)
    static void ApplySpatialAudio(
        const void* key,
        ALuint source, ALuint filter,
        ALuint envReverbEffect, ALuint envReverbSlot,
        const glm::vec3& position,
        bool environmental, bool disableSpatial, bool isUISound,
        bool artistFilterEnabled, float artistLowPassGain, float artistLowPassGainHF);

    // Called every frame a spatial-audio-eligible FMOD event instance is
    // updating (see FmodEventInstance::Update()). Attaches a private
    // "VaudioSpatialFilter" occlusion/ambient DSP + a private reverb DSP to
    // this instance's own channel group on first use (retrying lazily until
    // Studio actually creates one for it), and refreshes both every call.
    static void ApplySpatialAudio(
        const void* key, FMOD::Studio::EventInstance* instance,
        const glm::vec3& position,
        bool environmental, bool disableSpatial, bool isUISound);

    // Releases whatever spatial-audio state (vaudio emitter + FMOD DSPs, if
    // any) is associated with `key`. Safe to call unconditionally, including
    // for sounds that never had any (UI sounds, DisableSpatial, or a key that
    // was never registered) — always a no-op in that case.
    static void ReleaseSpatialAudio(const void* key);

private:
    static ALCdevice* device;

    static std::unordered_map<std::string, SoundBufferData>     loadedBuffers;
    static std::unordered_map<std::string, FMOD::Studio::Bank*> loadedBanks;

    /// Make `context` current, log driver info and HRTF status, set distance model.
    /// `label` is used in log messages to distinguish the two contexts.
    /// `expectHRTF` controls whether a warning is emitted when HRTF is inactive
    /// (pass false for the stereo context where HRTF is intentionally disabled).
    static void InitContext(ALCcontext* context, const char* label, bool expectHRTF);
    static void InitFmod();
    static void UpdateContext(ALCcontext* context);
    static void UpdateFmod();

    // Registers the custom "VaudioSpatialFilter" DSP plugin description with
    // coreSystem. Called once from InitFmod(). See SoundManager.cpp for the
    // plugin implementation.
    static void RegisterSpatialAudioDSPPlugin();
};
