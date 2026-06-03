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
};
