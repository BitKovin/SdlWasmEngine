#include "SoundManager.hpp"

#include <AL/alext.h>
#include "fmod_include.h"

#include "../FileSystem/FileSystem.h"
#include <Logger.hpp>

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
ALCdevice*  SoundManager::device        = nullptr;
ALCcontext* SoundManager::contextMono   = nullptr;
ALCcontext* SoundManager::contextStereo = nullptr;
float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume    = 1.0f;
float SoundManager::MusicVolume  = 1.0f;
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

    // Disable OpenAL's built-in distance attenuation. SoundInstance::ComputeDistanceGain()
    // handles falloff manually, so setting AL_NONE once here prevents double-attenuation.
    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
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
    if (contextMono)   { alcDestroyContext(contextMono);   contextMono   = nullptr; }
    if (device)        { alcCloseDevice(device);           device        = nullptr; }
}

void SoundManager::Update()
{
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
    const glm::vec3 up  = Camera::Up();
    const glm::vec3 vel = Camera::velocity;

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);

    const float orient[6] = { fwd.x, fwd.y, fwd.z, up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, orient);
}

void SoundManager::UpdateFmod()
{
    const glm::vec3 fwd = Camera::Forward();
    const glm::vec3 up  = Camera::Up();

    FMOD_3D_ATTRIBUTES attr;
    attr.position = { Camera::position.x, Camera::position.y, Camera::position.z };
    attr.velocity = { Camera::velocity.x, Camera::velocity.y, Camera::velocity.z };
    attr.forward  = { fwd.x, fwd.y, fwd.z };
    attr.up       = { up.x,  up.y,  up.z  };

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
    } catch (const std::exception& e) {
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
    Uint32 wavLen  = 0;
    Uint8* wavBuf  = nullptr;
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
        } else if (ch == 2) {
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

    const int    frameSize  = std::max(1, spec.channels * bytesPerSample);
    const Uint32 numFrames  = wavLen / static_cast<Uint32>(frameSize);
    const float  duration   = static_cast<float>(numFrames) / static_cast<float>(spec.freq);

    SoundBufferData data;
    data.buffer     = buffer;
    data.stereo     = isStereo;
    data.context    = isStereo ? contextStereo : contextMono;
    data.duration   = duration;
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

    FMOD::Studio::Bank* bank   = nullptr;
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
        } catch (const std::exception& e) {
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
