#include "SoundManager.hpp"

#include <AL/alext.h>
#include "fmod_include.h"

#include "../FileSystem/FileSystem.h"
#include "../SpatialSound/SpatialSoundManager.h"
#include <Logger.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>

#include <SpatialSound/SpatialSoundManager.h>

// ALC_SOFT_output_mode — define fallback values so the code compiles against
// older alext.h headers while still working correctly at runtime when a newer
// OpenAL Soft driver is present. These constants are stable across versions.
#ifndef ALC_OUTPUT_MODE_SOFT
#  define ALC_OUTPUT_MODE_SOFT  0x19AC
#endif
#ifndef ALC_STEREO_SOFT
#  define ALC_STEREO_SOFT       0x1501
#endif
#ifndef ALC_STEREO_HRTF_SOFT
#  define ALC_STEREO_HRTF_SOFT  0x19B2
#endif
#ifndef ALC_SURROUND_5_1_SOFT
#  define ALC_SURROUND_5_1_SOFT 0x1504
#endif
#ifndef ALC_SURROUND_6_1_SOFT
#  define ALC_SURROUND_6_1_SOFT 0x1505
#endif
#ifndef ALC_SURROUND_7_1_SOFT
#  define ALC_SURROUND_7_1_SOFT 0x1506
#endif

// ── Static member definitions ─────────────────────────────────────────────────
std::unordered_map<std::string, SoundBufferData>     SoundManager::loadedBuffers;
std::unordered_map<std::string, FMOD::Studio::Bank*> SoundManager::loadedBanks;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::contextMono = nullptr;
ALCcontext* SoundManager::contextStereo = nullptr;
float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume = 1.0f;
float SoundManager::MusicVolume = 1.0f;
float SoundManager::VoiceVolume = 1.0f;
float SoundManager::UiVolume = 1.0f;

// ── Helpers ───────────────────────────────────────────────────────────────────

#define FMOD_CHECK(x)                                                    \
    do {                                                                 \
        FMOD_RESULT _r = (x);                                            \
        if (_r != FMOD_OK) {                                             \
            Logger::Error("[FMOD] %s failed: %s (%d)",                   \
                          #x, FMOD_ErrorString(_r), _r);                \
            return;                                                      \
        }                                                                \
    } while (0)

// =============================================================================
// InitContext
// =============================================================================

void SoundManager::InitContext(ALCcontext* context, const char* label, bool expectHRTF)
{
    if (!context || alcMakeContextCurrent(context) == ALC_FALSE) {
        Logger::Error("[OpenAL] (%s) Failed to make context current", label);
        if (context) alcDestroyContext(context);
        alcCloseDevice(device);
        return;
    }

    Logger::Info("[OpenAL] (%s) Vendor:   %s", label, alGetString(AL_VENDOR));
    Logger::Info("[OpenAL] (%s) Renderer: %s", label, alGetString(AL_RENDERER));

    if (!alcIsExtensionPresent(device, "ALC_EXT_EFX"))
        Logger::Warning("[OpenAL] (%s) ALC_EXT_EFX not supported — effects unavailable", label);
    else {
        // We request 3 (Echo/Reverb/env-reverb sends) at context creation —
        // confirm the driver actually granted it. Some drivers cap this
        // lower than requested instead of failing context creation outright.
        ALCint grantedSends = 0;
        alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &grantedSends);
        if (grantedSends < 3)
            Logger::Warning("[OpenAL] (%s) Only %d auxiliary send(s) available — "
                "EnableReverb and/or the vaudio environment reverb "
                "(send index 2) will silently fail to route.",
                label, grantedSends);
    }

    if (!alIsExtensionPresent("AL_EXT_float32"))
        Logger::Warning("[OpenAL] (%s) AL_EXT_float32 not supported", label);

    // ── Output mode (ALC_SOFT_output_mode) ───────────────────────────────────
    // Logging the output mode is the fastest way to see why HRTF might not be
    // working — e.g. the system is in surround-sound mode.
    if (alcIsExtensionPresent(device, "ALC_SOFT_output_mode")) {
        ALCint outputMode = 0;
        alcGetIntegerv(device, ALC_OUTPUT_MODE_SOFT, 1, &outputMode);

        const char* modeStr = "unknown";
        switch (outputMode) {
        case ALC_STEREO_SOFT:       modeStr = "stereo";         break;
        case ALC_STEREO_HRTF_SOFT:  modeStr = "stereo HRTF";   break;
        case ALC_SURROUND_5_1_SOFT: modeStr = "5.1 surround";  break;
        case ALC_SURROUND_6_1_SOFT: modeStr = "6.1 surround";  break;
        case ALC_SURROUND_7_1_SOFT: modeStr = "7.1 surround";  break;
        }
        Logger::Info("[OpenAL] (%s) Output mode: %s (%d)", label, modeStr, outputMode);
    }

    // ── HRTF status (ALC_SOFT_HRTF) ──────────────────────────────────────────
    if (alcIsExtensionPresent(device, "ALC_SOFT_HRTF")) {
        ALCint hrtfStatus = ALC_HRTF_DISABLED_SOFT;
        alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &hrtfStatus);

        const char* statusStr = "unknown";
        switch (hrtfStatus) {
        case ALC_HRTF_DISABLED_SOFT:            statusStr = "disabled";              break;
        case ALC_HRTF_ENABLED_SOFT:             statusStr = "enabled";               break;
        case ALC_HRTF_DENIED_SOFT:              statusStr = "denied";                break;
        case ALC_HRTF_REQUIRED_SOFT:            statusStr = "required";              break;
        case ALC_HRTF_HEADPHONES_DETECTED_SOFT: statusStr = "enabled (headphones)";  break;
        case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT:  statusStr = "unsupported format";    break;
        }

        const ALCchar* hrtfName = alcGetString(device, ALC_HRTF_SPECIFIER_SOFT);
        Logger::Info("[OpenAL] (%s) HRTF status: %s | profile: %s",
            label, statusStr,
            (hrtfName && hrtfName[0]) ? hrtfName : "none");

        const bool hrtfActive = (hrtfStatus == ALC_HRTF_ENABLED_SOFT
            || hrtfStatus == ALC_HRTF_REQUIRED_SOFT
            || hrtfStatus == ALC_HRTF_HEADPHONES_DETECTED_SOFT);

        // Only warn when HRTF was actually requested. The stereo/2D context
        // intentionally runs without HRTF, so no warning is appropriate there.
        if (expectHRTF && !hrtfActive) {
            Logger::Warning("[OpenAL] (%s) HRTF is NOT active — 3D directionality will be reduced. "
                "Status: %s", label, statusStr);
        }
    }
    else if (expectHRTF) {
        Logger::Warning("[OpenAL] (%s) ALC_SOFT_HRTF extension not present on this device", label);
    }

    // Disable OpenAL's built-in distance attenuation. SoundInstance::GetDistanceFade()
    // (applied via AL_GAIN in UpdateSourceParams()) handles falloff manually, so AL_NONE
    // is required here to prevent double-attenuation.
    //
    // NOTE: this previously passed AL_LINEAR_DISTANCE_CLAMPED, which is an ACTIVE
    // distance model, not "no attenuation" - despite what the comment above always
    // claimed. With that model: the DIRECT path gets an extra distance-based cut on
    // top of the manual fade (scaled by AL_ROLLOFF_FACTOR, explicitly set to 1.0 in
    // UpdateSourceParams()), while each source's reverb aux SEND gets scaled by
    // AL_ROOM_ROLLOFF_FACTOR instead - which is never set anywhere in this codebase,
    // so it sat at OpenAL's own default of 0.0 (i.e. the send was never distance-
    // attenuated by the model at all). Net effect: the direct signal was attenuated
    // twice (manual fade x distance-model rolloff) while the reverb send was only
    // attenuated once, so as distance grows the direct path collapses toward silence
    // much faster than the reverb send — audible as "large room: the sound is far too
    // quiet, only its reverb is audible". AL_NONE turns off the distance model's own
    // attenuation entirely, so AL_GAIN (already carrying the correct, symmetric manual
    // fade for both the direct path and the aux send) is the only distance-based gain
    // applied to either path.
    alDistanceModel(AL_NONE);
}

// =============================================================================
// InitFmod
// =============================================================================

void SoundManager::InitFmod()
{
    FMOD_CHECK(FMOD::Studio::System::create(&studioSystem));
    studioSystem->getCoreSystem(&coreSystem);

    unsigned int coreFlags = FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS
        | FMOD_INIT_3D_RIGHTHANDED;

#if __EMSCRIPTEN__
    coreFlags |= FMOD_INIT_STREAM_FROM_UPDATE | FMOD_INIT_MIX_FROM_UPDATE;
    coreSystem->setDSPBufferSize(2048, 4);
    {
        unsigned int sz; int nb;
        coreSystem->getDSPBufferSize(&sz, &nb);
        Logger::Info("[FMOD] DSP buffer: size=%u, count=%d", sz, nb);
    }
#endif

    unsigned int studioFlags = FMOD_INIT_NORMAL;
#ifdef DEBUG
    studioFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif

    FMOD_CHECK(studioSystem->initialize(1024, studioFlags, coreFlags, nullptr));

    RegisterSpatialAudioDSPPlugin();
}

// =============================================================================
// Initialize / Close / Update
// =============================================================================

void SoundManager::Initialize()
{
    device = alcOpenDevice(nullptr);
    if (!device) {
        Logger::Error("[OpenAL] Failed to open audio device");
        return;
    }

    // ── Mono context (HRTF on): 3D spatial mono sources ──────────────────────
    {
        // ALC_OUTPUT_MODE_SOFT = ALC_STEREO_HRTF_SOFT forces stereo headphone
        // output regardless of the system's speaker configuration. Without it,
        // HRTF fails with ALC_HRTF_UNSUPPORTED_FORMAT_SOFT on surround setups
        // (5.1 / 7.1 etc.) because binaural processing requires a stereo signal.
        //
        // We build the attribute list dynamically so the extension is only added
        // when the driver actually supports it (runtime-checked, not compile-time).
        std::vector<ALCint> monoAttrs = {
            ALC_MONO_SOURCES,   256,
            ALC_STEREO_SOURCES,  64,
            ALC_HRTF_SOFT,      ALC_TRUE,
            // Sources on this context route up to 3 auxiliary sends:
            //   0 = artist EnableEcho, 1 = artist EnableReverb,
            //   2 = vaudio environment reverb (SoundManager::ApplySpatialAudio).
            // Without requesting this explicitly, the context falls back to
            // the driver/config default (commonly 1 or 2), which silently
            // drops alSource3i(..., AL_AUXILIARY_SEND_FILTER, ..., 2, ...)
            // for send index 2 (and sometimes 1) — no AL error is raised,
            // the effect just never gets connected. This is why EFX behaviour
            // has been inconsistent: it depends on the local driver's default.
            ALC_MAX_AUXILIARY_SENDS, 3,
        };

        if (alcIsExtensionPresent(device, "ALC_SOFT_output_mode")) {
            monoAttrs.push_back(ALC_OUTPUT_MODE_SOFT);
            monoAttrs.push_back(ALC_STEREO_HRTF_SOFT);
        }
        else {
            Logger::Warning("[OpenAL] ALC_SOFT_output_mode not available — "
                "HRTF may fail on surround-sound output devices");
        }

        monoAttrs.push_back(0); // terminator

        contextMono = alcCreateContext(device, monoAttrs.data());
        InitContext(contextMono, "Mono/3D", /*expectHRTF=*/true);
    }

    // ── Stereo context (HRTF off): non-spatial stereo / 2D / UI sources ──────
    // HRTF must be disabled here. Applying binaural head-related processing to
    // a pre-mixed stereo stream would fold the two channels together incorrectly,
    // producing distorted, out-of-phase output.
    {
        ALCint stereoAttrs[] = {
            ALC_MONO_SOURCES,    32,
            ALC_STEREO_SOURCES,  64,
            ALC_HRTF_SOFT,      ALC_FALSE,
            ALC_MAX_AUXILIARY_SENDS, 3, // see comment on monoAttrs above
            0
        };
        contextStereo = alcCreateContext(device, stereoAttrs);
        InitContext(contextStereo, "Stereo/2D", /*expectHRTF=*/false);
    }

    // Initialise the source pool now that both contexts exist.
    // This queries the real device limits and stores both context pointers so
    // pool operations never rely on "whatever is current".
    SoundInstance::InitPool(contextMono, contextStereo);

    InitFmod();
}

void SoundManager::Close()
{
    for (auto& [path, data] : loadedBuffers)
        alDeleteBuffers(1, &data.buffer);
    loadedBuffers.clear();

    alcMakeContextCurrent(nullptr);

    // Destroy both contexts — the original code accidentally leaked contextStereo.
    if (contextStereo) { alcDestroyContext(contextStereo); contextStereo = nullptr; }
    if (contextMono) { alcDestroyContext(contextMono);   contextMono = nullptr; }
    if (device) { alcCloseDevice(device);           device = nullptr; }
}

void SoundManager::Update()
{
    // Advance the spatial audio simulation first, so the occlusion/ambient
    // filter and reverb values both backends read below are fresh this frame.
    SpatialSoundManager::Update();

    UpdateContext(contextMono);
    UpdateContext(contextStereo);
    UpdateFmod();
}

// =============================================================================
// Context / FMOD per-frame updates
// =============================================================================

void SoundManager::UpdateContext(ALCcontext* context)
{
    alcMakeContextCurrent(context);

    const glm::vec3 pos = Camera::position;
    const glm::vec3 fwd = Camera::Forward();
    const glm::vec3 up = Camera::Up();
    const glm::vec3 vel = Camera::velocity;

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);

    const float orient[6] = { fwd.x, fwd.y, fwd.z, up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, orient);
}

void SoundManager::UpdateFmod()
{
    const glm::vec3 fwd = Camera::Forward();
    const glm::vec3 up = Camera::Up();

    FMOD_3D_ATTRIBUTES attr;
    attr.position = { Camera::position.x, Camera::position.y, Camera::position.z };
    attr.velocity = { Camera::velocity.x, Camera::velocity.y, Camera::velocity.z };
    attr.forward = { fwd.x, fwd.y, fwd.z };
    attr.up = { up.x,  up.y,  up.z };

    FMOD_RESULT r = studioSystem->setListenerAttributes(0, &attr);
    if (r != FMOD_OK)
        Logger::Error("[FMOD] setListenerAttributes: %s", FMOD_ErrorString(r));

    studioSystem->update();
}

// =============================================================================
// Asset management
// =============================================================================

SoundBufferData SoundManager::LoadOrGetSoundFileBuffer(const std::string& path)
{
    alGetError();

    auto it = loadedBuffers.find(path);
    if (it != loadedBuffers.end()) return it->second;

    // ── Read the file ─────────────────────────────────────────────────────────
    std::vector<uint8_t> fileData;
    try {
        fileData = FileSystemEngine::ReadFileBinary(path);
    }
    catch (const std::exception& e) {
        Logger::Error("[SoundManager] Failed to read '%s': %s", path.c_str(), e.what());
        return {};
    }

    SDL_RWops* rw = SDL_RWFromConstMem(fileData.data(), static_cast<int>(fileData.size()));
    if (!rw) {
        Logger::Error("[SoundManager] SDL_RWFromConstMem failed for '%s': %s",
            path.c_str(), SDL_GetError());
        return {};
    }

    SDL_AudioSpec spec = {};
    Uint32 wavLen = 0;
    Uint8* wavBuf = nullptr;
    if (!SDL_LoadWAV_RW(rw, /*freesrc=*/1, &spec, &wavBuf, &wavLen)) {
        Logger::Error("[SoundManager] SDL_LoadWAV_RW failed for '%s': %s",
            path.c_str(), SDL_GetError());
        return {};
    }

    Logger::Info("[SoundManager] '%s' — ch=%d fmt=0x%X freq=%d",
        path.c_str(), spec.channels, spec.format, spec.freq);

    // ── Resolve AL format ─────────────────────────────────────────────────────
    auto pickFormat = [](int ch, SDL_AudioFormat fmt) -> ALenum {
        if (ch == 1) {
            if (fmt == AUDIO_U8)                             return AL_FORMAT_MONO8;
            if (fmt == AUDIO_S16LSB || fmt == AUDIO_S16MSB) return AL_FORMAT_MONO16;
            if (fmt == AUDIO_F32LSB || fmt == AUDIO_F32MSB) return AL_FORMAT_MONO_FLOAT32;
        }
        else if (ch == 2) {
            if (fmt == AUDIO_U8)                             return AL_FORMAT_STEREO8;
            if (fmt == AUDIO_S16LSB || fmt == AUDIO_S16MSB) return AL_FORMAT_STEREO16;
            if (fmt == AUDIO_F32LSB || fmt == AUDIO_F32MSB) return AL_FORMAT_STEREO_FLOAT32;
        }
        return AL_NONE;
        };

    ALenum format = pickFormat(spec.channels, spec.format);
    if (format == AL_NONE) {
        Logger::Warning("[SoundManager] Unsupported format: ch=%d fmt=0x%X",
            spec.channels, spec.format);
        SDL_FreeWAV(wavBuf);
        return {};
    }

    // ── Upload to OpenAL ──────────────────────────────────────────────────────
    const bool isStereo = (spec.channels > 1);
    alcMakeContextCurrent(isStereo ? contextStereo : contextMono);

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Error("[SoundManager] alGenBuffers error: 0x%X", err);
        SDL_FreeWAV(wavBuf);
        return {};
    }

    alBufferData(buffer, format, wavBuf, static_cast<ALsizei>(wavLen), spec.freq);
    if (ALenum err = alGetError(); err != AL_NO_ERROR) {
        Logger::Error("[SoundManager] alBufferData error: 0x%X", err);
        alDeleteBuffers(1, &buffer);
        SDL_FreeWAV(wavBuf);
        return {};
    }

    SDL_FreeWAV(wavBuf);

    // ── Compute duration ──────────────────────────────────────────────────────
    int bytesPerSample;
    switch (spec.format) {
    case AUDIO_U8:                         bytesPerSample = 1; break;
    case AUDIO_S16LSB: case AUDIO_S16MSB:  bytesPerSample = 2; break;
    case AUDIO_F32LSB: case AUDIO_F32MSB:  bytesPerSample = 4; break;
    default:
        Logger::Warning("[SoundManager] Unknown format for duration calc: 0x%X", spec.format);
        alDeleteBuffers(1, &buffer);
        return {};
    }

    const int    frameSize = std::max(1, spec.channels * bytesPerSample);
    const Uint32 numFrames = wavLen / static_cast<Uint32>(frameSize);
    const float  duration = static_cast<float>(numFrames) / static_cast<float>(spec.freq);

    SoundBufferData data;
    data.buffer = buffer;
    data.stereo = isStereo;
    data.context = isStereo ? contextStereo : contextMono;
    data.duration = duration;
    data.sampleRate = static_cast<ALuint>(spec.freq);

    loadedBuffers[path] = data;
    return data;
}

std::shared_ptr<SoundInstance> SoundManager::GetSoundFromPath(const std::string& path)
{
    SoundBufferData data = LoadOrGetSoundFileBuffer(path);
    if (!data.IsValid()) return nullptr;
    return std::make_shared<SoundInstance>(data);
}

void SoundManager::CleanAllData()
{
    for (auto& [path, data] : loadedBuffers)
        alDeleteBuffers(1, &data.buffer);
    loadedBuffers.clear();

    for (auto& [path, bank] : loadedBanks) {
        if (!bank) continue;
        bank->unloadSampleData();
        bank->unload();
    }
    loadedBanks.clear();
}

// =============================================================================
// FMOD bank loading
// =============================================================================

FMOD::Studio::Bank* SoundManager::LoadBankFromPath(const std::string& bankPath,
    bool loadSampleData)
{
    if (!studioSystem) {
        Logger::Error("[FMOD] LoadBankFromPath: studioSystem is null");
        return nullptr;
    }

    auto it = loadedBanks.find(bankPath);
    if (it != loadedBanks.end()) return it->second;

    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT         result = studioSystem->loadBankFile(
        bankPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

    if (result == FMOD_OK && bank) {
        Logger::Info("[FMOD] Bank loaded from disk: %s", bankPath.c_str());
    }
    else {
        Logger::Warning("[FMOD] loadBankFile failed for '%s' (%d: %s) — trying memory load",
            bankPath.c_str(), result, FMOD_ErrorString(result));

        std::vector<uint8_t> bankData;
        try {
            bankData = FileSystemEngine::ReadFileBinary(bankPath);
        }
        catch (const std::exception& e) {
            Logger::Error("[FMOD] Failed to read '%s': %s", bankPath.c_str(), e.what());
            return loadedBanks[bankPath] = nullptr;
        }

        result = studioSystem->loadBankMemory(
            reinterpret_cast<const char*>(bankData.data()),
            static_cast<int>(bankData.size()),
            FMOD_STUDIO_LOAD_MEMORY,
            FMOD_STUDIO_LOAD_BANK_NORMAL,
            &bank);

        if (result != FMOD_OK || !bank) {
            Logger::Error("[FMOD] Memory load failed for '%s' (%d: %s)",
                bankPath.c_str(), result, FMOD_ErrorString(result));
            return loadedBanks[bankPath] = nullptr;
        }

        Logger::Info("[FMOD] Bank loaded from memory: %s", bankPath.c_str());
    }

    if (loadSampleData) {
        result = bank->loadSampleData();
        if (result != FMOD_OK)
            Logger::Warning("[FMOD] Sample data load failed for '%s' (%d: %s)",
                bankPath.c_str(), result, FMOD_ErrorString(result));
        else
            Logger::Info("[FMOD] Sample data loaded: %s", bankPath.c_str());
    }

    return loadedBanks[bankPath] = bank;
}

float SoundManager::GetVolumeForSoundType(SoundType type)
{
    switch (type)
    {
    case SoundType::SFX:
        return SfxVolume;
        break;
    case SoundType::Music:
        return MusicVolume;
        break;
    case SoundType::Voice:
        return VoiceVolume;
        break;
    case SoundType::UI:
        return UiVolume;
        break;
    default:
        return 1.0f;
        break;
    }
}

// =============================================================================
// Spatial audio (vaudio) integration
//
// This is the only part of SoundManager.cpp that's aware of the spatial
// audio backend — it talks to SpatialSoundManager's plain, engine-owned
// types (SpatialLowPassFilter, SpatialReverb, ...) and never touches vaudio.h
// itself. See SpatialSound/SpatialSoundManager.h for the full rationale.
//
// All effects are per sound instance: every SoundInstance/FmodEventInstance
// gets its own emitter and its own private DSP/filter state, keyed by the
// `key` pointer the caller passes in (their own `this`). Nothing here is
// shared across simultaneous sounds — mirroring the vaudio OpenAL/FMOD
// examples, where every "speech"/channel got its own filter and reverb
// effect rather than one shared instance.
// =============================================================================

namespace
{
    struct SpatialAudioState
    {
        SpatialSoundEmitter* emitter = nullptr; // vaudio-side positional emitter; null for `environmental` sounds

        // FMOD-only — lazily created the first time this key's ApplySpatialAudio
        // (FMOD overload) call finds a usable channel group.
        FMOD::ChannelGroup* channelGroup = nullptr;
        FMOD::DSP* occlusionFilter = nullptr; // "VaudioSpatialFilter" plugin instance (see below)
        FMOD::DSP* reverb = nullptr; // built-in SFXREVERB instance
    };

    std::unordered_map<const void*, SpatialAudioState> g_spatialAudio;

    // ─────────────────────────────────────────────────────────────────────────
    // "VaudioSpatialFilter" — a small custom native FMOD DSP plugin.
    //
    // Applies the same two-parameter model as OpenAL's AL_FILTER_LOWPASS
    // (overall linear Gain, plus GainHF controlling how much high-frequency
    // content passes through a one-pole low-pass), so both audio backends
    // respond identically to the same SpatialLowPassFilter data — used for
    // both per-target occlusion (positional sounds) and the ambient filter
    // (EnvironmentalSound instances).
    //
    // NOTE: FMOD_DSP_DESCRIPTION's exact field layout/callback signatures have
    // been stable across many FMOD versions, but can vary slightly between
    // SDK releases — cross-check this against your installed FMOD low-level
    // SDK's fmod_dsp.h before shipping.
    // ─────────────────────────────────────────────────────────────────────────

    enum VaudioFilterParam
    {
        VAUDIO_FILTER_PARAM_GAIN = 0,
        VAUDIO_FILTER_PARAM_GAINHF = 1,
        VAUDIO_FILTER_NUM_PARAMS
    };

    struct VaudioFilterState
    {
        float gain = 1.0f;
        float gainHF = 1.0f;
        static constexpr int kMaxChannels = 8;
        float history[kMaxChannels] = {};
    };

    FMOD_RESULT F_CALLBACK VaudioFilter_Create(FMOD_DSP_STATE* dspState)
    {
        void* mem = dspState->functions->alloc(sizeof(VaudioFilterState), FMOD_MEMORY_NORMAL, "VaudioSpatialFilter");
        if (!mem) return FMOD_ERR_MEMORY;

        dspState->plugindata = new (mem) VaudioFilterState();
        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK VaudioFilter_Release(FMOD_DSP_STATE* dspState)
    {
        if (VaudioFilterState* state = (VaudioFilterState*)dspState->plugindata)
        {
            state->~VaudioFilterState();
            dspState->functions->free(state, FMOD_MEMORY_NORMAL, "VaudioSpatialFilter");
        }
        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK VaudioFilter_Read(FMOD_DSP_STATE* dspState, float* inbuffer, float* outbuffer,
        unsigned int length, int inchannels, int* outchannels)
    {
        VaudioFilterState* state = (VaudioFilterState*)dspState->plugindata;
        *outchannels = inchannels;

        int sampleRate = 48000;
        if (dspState->functions && dspState->functions->getsamplerate)
            dspState->functions->getsamplerate(dspState, &sampleRate);

        // Fixed corner frequency for the one-pole filter driving GainHF's
        // blend — matches the informal "reference frequency" OpenAL's simple
        // lowpass model implies. Not exposed as a parameter; vaudio only ever
        // gives us a single GainHF value to work with, same as OpenAL does.
        const float cutoffHz = 5000.0f;
        const float pole = 1.0f - std::exp(-2.0f * 3.14159265f * cutoffHz / (float)sampleRate);

        const int trackedChannels = std::min(inchannels, VaudioFilterState::kMaxChannels);

        for (unsigned int i = 0; i < length; i++)
        {
            for (int ch = 0; ch < inchannels; ch++)
            {
                float in = inbuffer[i * inchannels + ch];
                float out;

                if (ch < trackedChannels)
                {
                    float& hist = state->history[ch];
                    hist += pole * (in - hist);
                    // GainHF blends between the filtered (0) and dry (1) signal —
                    // 1.0 = fully open, 0.0 = fully filtered.
                    out = hist + state->gainHF * (in - hist);
                }
                else
                {
                    out = in; // beyond our tracked channel count — pass through unfiltered
                }

                outbuffer[i * inchannels + ch] = out * state->gain;
            }
        }

        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK VaudioFilter_SetParamFloat(FMOD_DSP_STATE* dspState, int index, float value)
    {
        VaudioFilterState* state = (VaudioFilterState*)dspState->plugindata;
        switch (index)
        {
        case VAUDIO_FILTER_PARAM_GAIN:   state->gain = value; return FMOD_OK;
        case VAUDIO_FILTER_PARAM_GAINHF: state->gainHF = value; return FMOD_OK;
        default: return FMOD_ERR_INVALID_PARAM;
        }
    }

    FMOD_RESULT F_CALLBACK VaudioFilter_GetParamFloat(FMOD_DSP_STATE* dspState, int index, float* value, char*)
    {
        VaudioFilterState* state = (VaudioFilterState*)dspState->plugindata;
        switch (index)
        {
        case VAUDIO_FILTER_PARAM_GAIN:   *value = state->gain;   return FMOD_OK;
        case VAUDIO_FILTER_PARAM_GAINHF: *value = state->gainHF; return FMOD_OK;
        default: return FMOD_ERR_INVALID_PARAM;
        }
    }

    FMOD_DSP_PARAMETER_DESC g_paramGain{};
    FMOD_DSP_PARAMETER_DESC g_paramGainHF{};
    FMOD_DSP_PARAMETER_DESC* g_paramList[VAUDIO_FILTER_NUM_PARAMS] = {};
    FMOD_DSP_DESCRIPTION     g_filterDescription{};

    // Builds a SpatialLowPassFilter for `key`'s current situation: the
    // per-target occlusion filter for positional sounds, or the ambient
    // filter for EnvironmentalSound instances (see SpatialSoundManager.h).
    SpatialLowPassFilter ResolveFilter(SpatialAudioState& state, bool environmental)
    {
        if (environmental) return SpatialSoundManager::GetAmbientFilter();
        if (state.emitter) return SpatialSoundManager::GetOcclusionFilter(state.emitter);
        return SpatialLowPassFilter{};
    }
}

void SoundManager::RegisterSpatialAudioDSPPlugin()
{
    FMOD_DSP_INIT_PARAMDESC_FLOAT(g_paramGain, "Gain", "", "Overall linear gain (0-1)", 0.0f, 1.0f, 1.0f);
    FMOD_DSP_INIT_PARAMDESC_FLOAT(g_paramGainHF, "GainHF", "", "High-frequency pass-through (0-1)", 0.0f, 1.0f, 1.0f);
    g_paramList[VAUDIO_FILTER_PARAM_GAIN] = &g_paramGain;
    g_paramList[VAUDIO_FILTER_PARAM_GAINHF] = &g_paramGainHF;

    g_filterDescription.pluginsdkversion = FMOD_PLUGIN_SDK_VERSION;
    std::strncpy(g_filterDescription.name, "VaudioSpatialFilter", sizeof(g_filterDescription.name) - 1);
    g_filterDescription.version = 0x00010000;
    g_filterDescription.numinputbuffers = 1;
    g_filterDescription.numoutputbuffers = 1;
    g_filterDescription.create = VaudioFilter_Create;
    g_filterDescription.release = VaudioFilter_Release;
    g_filterDescription.read = VaudioFilter_Read;
    g_filterDescription.numparameters = VAUDIO_FILTER_NUM_PARAMS;
    g_filterDescription.paramdesc = g_paramList;
    g_filterDescription.setparameterfloat = VaudioFilter_SetParamFloat;
    g_filterDescription.getparameterfloat = VaudioFilter_GetParamFloat;

    Logger::Info("[SoundManager] VaudioSpatialFilter DSP description ready");
}

// ── OpenAL ───────────────────────────────────────────────────────────────────

void SoundManager::ApplySpatialAudio(
    const void* key,
    ALuint source, ALuint filter,
    ALuint envReverbEffect, ALuint envReverbSlot,
    const glm::vec3& position,
    bool environmental, bool disableSpatial, bool isUISound,
    bool artistFilterEnabled, float artistLowPassGain, float artistLowPassGainHF)
{
    if (!key || isUISound || disableSpatial) return;

    SpatialAudioState& state = g_spatialAudio[key]; // per-instance — never shared

    if (!environmental)
    {
        if (!state.emitter) state.emitter = SpatialSoundManager::CreateSoundEmitter(position);
        else                 SpatialSoundManager::UpdateSoundEmitterPosition(state.emitter, position);
    }

    if (source && filter)
    {
        SpatialLowPassFilter occlusion = ResolveFilter(state, environmental);

        float gainLF = occlusion.gainLF;
        float gainHF = occlusion.gainHF;
        if (artistFilterEnabled)
        {
            gainLF *= artistLowPassGain;
            gainHF *= artistLowPassGainHF;
        }

        alFilterf(filter, AL_LOWPASS_GAIN, gainLF);
        alFilterf(filter, AL_LOWPASS_GAINHF, gainHF);
        alSourcei(source, AL_DIRECT_FILTER, filter);
    }

    SpatialReverb reverb;
    if (source && envReverbEffect && envReverbSlot && SpatialSoundManager::GetListenerReverb(reverb))
    {
        ALint effectType = AL_EFFECT_REVERB;
#ifdef AL_EFFECT_EAXREVERB
        effectType = AL_EFFECT_EAXREVERB;
#endif
        alEffecti(envReverbEffect, AL_EFFECT_TYPE, effectType);

#ifdef AL_EFFECT_EAXREVERB
        alEffectf(envReverbEffect, AL_EAXREVERB_DENSITY, reverb.density);
        alEffectf(envReverbEffect, AL_EAXREVERB_DIFFUSION, reverb.diffusion);
        alEffectf(envReverbEffect, AL_EAXREVERB_GAIN, reverb.gain);
        alEffectf(envReverbEffect, AL_EAXREVERB_GAINHF, reverb.gainHF);
        alEffectf(envReverbEffect, AL_EAXREVERB_GAINLF, reverb.gainLF);
        alEffectf(envReverbEffect, AL_EAXREVERB_DECAY_TIME, reverb.decayTime);
        alEffectf(envReverbEffect, AL_EAXREVERB_DECAY_HFRATIO, reverb.decayHFRatio);
        alEffectf(envReverbEffect, AL_EAXREVERB_DECAY_LFRATIO, reverb.decayLFRatio);
        alEffectf(envReverbEffect, AL_EAXREVERB_REFLECTIONS_GAIN, reverb.reflectionsGain);
        alEffectf(envReverbEffect, AL_EAXREVERB_REFLECTIONS_DELAY, reverb.reflectionsDelay);
        alEffectf(envReverbEffect, AL_EAXREVERB_LATE_REVERB_GAIN, reverb.lateReverbGain);
        alEffectf(envReverbEffect, AL_EAXREVERB_LATE_REVERB_DELAY, reverb.lateReverbDelay);
        alEffectf(envReverbEffect, AL_EAXREVERB_ECHO_TIME, reverb.echoTime);
        alEffectf(envReverbEffect, AL_EAXREVERB_ECHO_DEPTH, reverb.echoDepth);
        alEffectf(envReverbEffect, AL_EAXREVERB_MODULATION_TIME, reverb.modulationTime);
        alEffectf(envReverbEffect, AL_EAXREVERB_MODULATION_DEPTH, reverb.modulationDepth);
        alEffectf(envReverbEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb.airAbsorptionGainHF);
        alEffectf(envReverbEffect, AL_EAXREVERB_HFREFERENCE, reverb.hfReference);
        alEffectf(envReverbEffect, AL_EAXREVERB_LFREFERENCE, reverb.lfReference);
        alEffectf(envReverbEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb.roomRolloffFactor);
        alEffecti(envReverbEffect, AL_EAXREVERB_DECAY_HFLIMIT, reverb.decayHFLimit);

        // Directional reverb (see SpatialReverb::direction / vaudio's
        // Grouped EAX "relative direction" feature). Both PAN properties
        // take the same vector per the vaudio docs — reflectionsPan and
        // lateReverbPan are set identically from one direction value.
        const ALfloat pan[3] = { reverb.direction.x, reverb.direction.y, reverb.direction.z };
        alEffectfv(envReverbEffect, AL_EAXREVERB_REFLECTIONS_PAN, pan);
        alEffectfv(envReverbEffect, AL_EAXREVERB_LATE_REVERB_PAN, pan);
#else
        // AL_EFFECT_EAXREVERB isn't available on this platform's EFX headers —
        // fall back to the plain AL_EFFECT_REVERB parameter set. Note: base
        // AL_EFFECT_REVERB has no PAN equivalent, so directionality is lost
        // entirely on this fallback path — only EAXReverb supports it.
        alEffectf(envReverbEffect, AL_REVERB_DENSITY, reverb.density);
        alEffectf(envReverbEffect, AL_REVERB_DIFFUSION, reverb.diffusion);
        alEffectf(envReverbEffect, AL_REVERB_GAIN, reverb.gain);
        alEffectf(envReverbEffect, AL_REVERB_GAINHF, reverb.gainHF);
        alEffectf(envReverbEffect, AL_REVERB_DECAY_TIME, reverb.decayTime);
        alEffectf(envReverbEffect, AL_REVERB_DECAY_HFRATIO, reverb.decayHFRatio);
        alEffectf(envReverbEffect, AL_REVERB_REFLECTIONS_GAIN, reverb.reflectionsGain);
        alEffectf(envReverbEffect, AL_REVERB_REFLECTIONS_DELAY, reverb.reflectionsDelay);
        alEffectf(envReverbEffect, AL_REVERB_LATE_REVERB_GAIN, reverb.lateReverbGain);
        alEffectf(envReverbEffect, AL_REVERB_LATE_REVERB_DELAY, reverb.lateReverbDelay);
        alEffectf(envReverbEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb.airAbsorptionGainHF);
        alEffectf(envReverbEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb.roomRolloffFactor);
        alEffecti(envReverbEffect, AL_REVERB_DECAY_HFLIMIT, reverb.decayHFLimit);
#endif
        alAuxiliaryEffectSloti(envReverbSlot, AL_EFFECTSLOT_EFFECT, envReverbEffect);
        alSource3i(source, AL_AUXILIARY_SEND_FILTER, envReverbSlot, 2, AL_FILTER_NULL);
    }
}

// ── FMOD ─────────────────────────────────────────────────────────────────────

void SoundManager::ApplySpatialAudio(
    const void* key, FMOD::Studio::EventInstance* instance,
    const glm::vec3& position,
    bool environmental, bool disableSpatial, bool isUISound)
{
    if (!key || !instance || isUISound || disableSpatial) return;

    SpatialAudioState& state = g_spatialAudio[key]; // per-instance — never shared

    if (!environmental)
    {
        if (!state.emitter) state.emitter = SpatialSoundManager::CreateSoundEmitter(position);
        else                 SpatialSoundManager::UpdateSoundEmitterPosition(state.emitter, position);
    }

    // Studio event instances only expose their internal ChannelGroup once the
    // voice has actually been created (after start() + at least one
    // Studio::System::update()) — keep retrying lazily until it succeeds.
    if (!state.channelGroup)
    {
        instance->getChannelGroup(&state.channelGroup);
        if (!state.channelGroup) return;
    }

    // ── Occlusion / ambient filter — private "VaudioSpatialFilter" DSP ───────
    if (!state.occlusionFilter)
    {
        if (coreSystem->createDSP(&g_filterDescription, &state.occlusionFilter) == FMOD_OK && state.occlusionFilter)
            state.channelGroup->addDSP(0, state.occlusionFilter);
    }

    if (state.occlusionFilter)
    {
        SpatialLowPassFilter occlusion = ResolveFilter(state, environmental);
        state.occlusionFilter->setParameterFloat(VAUDIO_FILTER_PARAM_GAIN, occlusion.gainLF);
        state.occlusionFilter->setParameterFloat(VAUDIO_FILTER_PARAM_GAINHF, occlusion.gainHF);
    }

    // ── Reverb — private built-in SFXREVERB instance ──────────────────────────
    SpatialReverb reverb;
    if (SpatialSoundManager::GetListenerReverb(reverb))
    {
        if (!state.reverb)
        {
            if (coreSystem->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &state.reverb) == FMOD_OK && state.reverb)
                state.channelGroup->addDSP(0, state.reverb);
        }

        if (state.reverb)
        {
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, reverb.decayTime * 1000.0f);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, reverb.reflectionsDelay * 1000.0f);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, reverb.lateReverbDelay * 1000.0f);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, reverb.hfReference);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, std::clamp(reverb.decayHFRatio * 100.0f, 10.0f, 100.0f));
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, reverb.diffusion * 100.0f);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, reverb.density * 100.0f);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFFREQUENCY, reverb.lfReference);
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFGAIN, 20.0f * std::log10(std::max(reverb.gainLF, 1e-6f)));
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, reverb.hfReference * reverb.airAbsorptionGainHF);

            float totalGain = reverb.reflectionsGain + reverb.lateReverbGain;
            float earlyLateMix = totalGain > 0.0f ? reverb.reflectionsGain / totalGain * 100.0f : 50.0f;
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, std::clamp(earlyLateMix, 0.0f, 100.0f));

            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, 20.0f * std::log10(std::max((reverb.gainLF + reverb.gainHF) * 0.5f, 1e-6f)));
            state.reverb->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, 0.0f);
        }
    }
}

// ── Shared release ───────────────────────────────────────────────────────────

void SoundManager::ReleaseSpatialAudio(const void* key)
{
    auto it = g_spatialAudio.find(key);
    if (it == g_spatialAudio.end()) return;

    SpatialAudioState& state = it->second;

    if (state.occlusionFilter)
    {
        if (state.channelGroup) state.channelGroup->removeDSP(state.occlusionFilter);
        state.occlusionFilter->release();
    }
    if (state.reverb)
    {
        if (state.channelGroup) state.channelGroup->removeDSP(state.reverb);
        state.reverb->release();
    }

    SpatialSoundManager::ReleaseSoundEmitter(state.emitter);

    g_spatialAudio.erase(it);
}