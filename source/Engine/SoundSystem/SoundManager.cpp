#include "SoundManager.hpp"

#include <AL/alext.h>
#include "fmod_include.h"

#include "../FileSystem/FileSystem.h"
#include <Logger.hpp>

std::unordered_map<std::string, SoundBufferData> SoundManager::loadedBuffers;
std::unordered_map<std::string, FMOD::Studio::Bank*> SoundManager::loadedBanks;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::contextMono = nullptr;
ALCcontext* SoundManager::contextStereo = nullptr;

float SoundManager::GlobalVolume = 0.3f;
float SoundManager::SfxVolume = 1.0f; //unused after FMOD implementation
float SoundManager::MusicVolume = 1.0f; //unused after FMOD implementation

void SoundManager::InitContext(ALCcontext* context)
{
    if (!context || alcMakeContextCurrent(context) == ALC_FALSE) {
        Logger::Log("Failed to create or make current OpenAL context.\n");
        if (context) alcDestroyContext(context);
        alcCloseDevice(device);
        return;
    }

    Logger::Info("Vendor:   %s", alGetString(AL_VENDOR));
    Logger::Info("Renderer: %s", alGetString(AL_RENDERER));

    if (!alcIsExtensionPresent(device, "ALC_SOFT_HRTF"))
        Logger::Warning("ALC_SOFT_HRTF not supported");

    if (!alcIsExtensionPresent(device, "ALC_EXT_EFX"))
        Logger::Warning("ALC_EXT_EFX not supported");

    ALboolean hasFloat32 = alIsExtensionPresent("AL_EXT_float32");
    if (!hasFloat32)
        Logger::Warning("AL_EXT_float32 not supported");

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
}

#define FMOD_CHECK(x)                                                     \
    do {                                                                  \
        FMOD_RESULT result = (x);                                         \
        if (result != FMOD_OK) {                                          \
            Logger::Error("[FMOD ERROR] %s failed: %s (%d)",              \
                   #x, FMOD_ErrorString(result), result);                \
            return;                                                       \
        }                                                                 \
    } while (0)


void SoundManager::InitFmod()
{
    FMOD_CHECK(FMOD::Studio::System::create(&studioSystem));


    unsigned int flags = FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS;
    flags |= FMOD_INIT_3D_RIGHTHANDED;



    studioSystem->getCoreSystem(&coreSystem);

    //coreSystem->setDSPBufferSize(512, 4);

#if __EMSCRIPTEN__

    flags |= FMOD_INIT_STREAM_FROM_UPDATE | FMOD_INIT_MIX_FROM_UPDATE ;


    coreSystem->setDSPBufferSize(2048, 4);

    unsigned int size;
    int num;

    coreSystem->getDSPBufferSize(&size, &num);


    Logger::Info("DSP Buffer Size: %u, Number of Buffers: %d", size, num);


    //coreSystem->setSoftwareFormat(96000, FMOD_SPEAKERMODE::FMOD_SPEAKERMODE_STEREO, 0);

#endif // _EMSCRIPTEN_


    unsigned int studioFlags = FMOD_INIT_NORMAL;

#ifdef DEBUG
    studioFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif

	int maxSounds = 512;

#ifdef __EMSCRIPTEN__

    maxSounds = 128;

#endif // __EMSCRIPTEN__


    FMOD_CHECK(studioSystem->initialize(
        1024,               // max channels
        studioFlags,
        flags,
        nullptr
    ));

    /*

    unsigned int version;

    coreSystem->getVersion(&version);

    int major = (version >> 16) & 0xFFFF;
    int minor = (version >> 8) & 0xFF;
    int patch = version & 0xFF;

    std::cout << "FMOD version: "
        << major << "."
        << minor << "."
        << patch << std::endl;
        */
}

void SoundManager::UpdateContext(ALCcontext* context)
{
    alcMakeContextCurrent(context);

    vec3 camPos = Camera::position;
    vec3 camForward = Camera::Forward();
    vec3 camUp = Camera::Up();
    vec3 camVel = Camera::velocity;

    alListener3f(AL_POSITION, camPos.x, camPos.y, camPos.z);
    alListener3f(AL_VELOCITY, camVel.x, camVel.y, camVel.z);

    float orient[6] = {
        camForward.x, camForward.y, camForward.z,
        camUp.x,      camUp.y,      camUp.z
    };
    alListenerfv(AL_ORIENTATION, orient);

}

void SoundManager::UpdateFmod()
{

    vec3 camForward = Camera::Forward();
    vec3 camUp = Camera::Up();

    // 1) Build your FMOD_VECTORs
    FMOD_VECTOR position = { Camera::position.x, Camera::position.y, Camera::position.z };
    FMOD_VECTOR velocity = { Camera::velocity.x, Camera::velocity.y, Camera::velocity.z };
    FMOD_VECTOR forward = { camForward.x, camForward.y, camForward.z };
    FMOD_VECTOR up = { camUp.x,      camUp.y,      camUp.z };

    // 2) Pack into the attributes struct
    FMOD_3D_ATTRIBUTES listenerAttr = { position, velocity, forward, up };

    // 3) Call the Studio API
    FMOD_RESULT result = studioSystem->setListenerAttributes(0, &listenerAttr);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD Studio setListenerAttributes failed ("
            << result << "): " << FMOD_ErrorString(result) << "\n";
    }

    studioSystem->update();

}

void SoundManager::Initialize()
{
    device = alcOpenDevice(nullptr);
    if (!device) {
        Logger::Log("Failed to open OpenAL device.\n");
        return;
    }

    ALCint ctxAttrsMono[] = {
        ALC_MONO_SOURCES,      256,
        ALC_STEREO_SOURCES,     64,
        ALC_HRTF_SOFT,       ALC_TRUE,
        //ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
        0
    };
    contextMono = alcCreateContext(device, ctxAttrsMono);

    InitContext(contextMono);


    ALCint ctxAttrsStereo[] = {
    ALC_MONO_SOURCES,      32,
    ALC_STEREO_SOURCES,     64,
    ALC_HRTF_SOFT,       ALC_FALSE,
    //ALC_OUTPUT_LIMITER_SOFT, ALC_FALSE,
    0
    };
    contextStereo = alcCreateContext(device, ctxAttrsStereo);

    InitContext(contextStereo);

    InitFmod();


}

void SoundManager::Close()
{

    for (auto& kv : loadedBuffers) {
        alDeleteBuffers(1, &kv.second.buffer);
    }
    loadedBuffers.clear();

    alcMakeContextCurrent(nullptr);
    if (contextMono) alcDestroyContext(contextMono);
    contextMono = nullptr;
    if (device) alcCloseDevice(device);
    device = nullptr;

}

void SoundManager::Update()
{
    UpdateContext(contextMono);
    UpdateContext(contextStereo);

    UpdateFmod();

}

SoundBufferData SoundManager::LoadOrGetSoundFileBuffer(std::string path)
{
    alGetError();

    auto found = loadedBuffers.find(path);
    if (found != loadedBuffers.end()) {
        return found->second;
    }

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    // 1) pull the .wav into RAM
    std::vector<uint8_t> fileData;
    try {
        fileData = FileSystemEngine::ReadFileBinary(path);
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to read WAV '%s': %s", path.c_str(), e.what());
        return SoundBufferData();
    }

    // 2) wrap it in an SDL_RWops
    SDL_RWops* rw = SDL_RWFromConstMem(fileData.data(), static_cast<int>(fileData.size()));
    if (!rw) {
        Logger::Error("SDL_RWFromConstMem failed for '%s': %s", path.c_str(), SDL_GetError());
        return SoundBufferData();
    }

    // 3) load WAV from that RWops, freeing the RWops on success/failure
    if (SDL_LoadWAV_RW(rw, /*freesrc=*/1, &wavSpec, &wavBuffer, &wavLength) == nullptr) {
        Logger::Error("Failed to decode WAV '%s': %s", path.c_str(), SDL_GetError());
        return SoundBufferData();
    }

    Logger::Info("Loading WAV: channels=%d, format=%d, freq=%d",
        wavSpec.channels, wavSpec.format, wavSpec.freq);

    bool isStereo = wavSpec.channels > 1;
    alcMakeContextCurrent(isStereo ? contextStereo : contextMono);

    ALenum format;

    if (wavSpec.channels == 1) {
        if (wavSpec.format == AUDIO_U8)
            format = AL_FORMAT_MONO8;
        else if (wavSpec.format == AUDIO_S16LSB || wavSpec.format == AUDIO_S16MSB)
            format = AL_FORMAT_MONO16;
        else if (wavSpec.format == AUDIO_F32LSB || wavSpec.format == AUDIO_F32MSB)
            format = AL_FORMAT_MONO_FLOAT32;
        else {
            Logger::Warning("Unsupported format: %d", wavSpec.format);
            SDL_FreeWAV(wavBuffer);
            return SoundBufferData();
        }
    }
    else if (wavSpec.channels == 2) {
        if (wavSpec.format == AUDIO_U8)
            format = AL_FORMAT_STEREO8;
        else if (wavSpec.format == AUDIO_S16LSB || wavSpec.format == AUDIO_S16MSB)
            format = AL_FORMAT_STEREO16;
        else if (wavSpec.format == AUDIO_F32LSB || wavSpec.format == AUDIO_F32MSB)
            format = AL_FORMAT_STEREO_FLOAT32;
        else {
            Logger::Warning("Unsupported format: %d", wavSpec.format);
            SDL_FreeWAV(wavBuffer);
            return SoundBufferData();
        }
    }
    else {
        Logger::Warning("Unsupported channel count: %d", wavSpec.channels);
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        Logger::Error("Error generating buffer: %s", alGetString(err));
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    alBufferData(buffer, format, wavBuffer, wavLength, wavSpec.freq);
    err = alGetError();
    if (err != AL_NO_ERROR) {
        Logger::Error("Error setting buffer data: %s", alGetString(err));
        alDeleteBuffers(1, &buffer);
        SDL_FreeWAV(wavBuffer);
        return SoundBufferData();
    }

    SDL_FreeWAV(wavBuffer);



    SoundBufferData data;
    data.buffer = buffer;
    data.stereo = isStereo;
    data.context = isStereo ? contextStereo : contextMono;
    
    int bytesPerSample;
    switch (wavSpec.format) {
    case AUDIO_U8:       bytesPerSample = 1; break;
    case AUDIO_S16LSB:
    case AUDIO_S16MSB:   bytesPerSample = 2; break;
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:   bytesPerSample = 4; break;
    default:
        Logger::Warning("Unknown format for duration calculation: %d", wavSpec.format);
        alDeleteBuffers(1, &buffer);
        return SoundBufferData();
    }

    int frameSize = wavSpec.channels * bytesPerSample;
    if (frameSize == 0) frameSize = 1; // safety

    Uint32 numSamples = wavLength / frameSize;
    float duration = static_cast<float>(numSamples) / wavSpec.freq;
    data.duration = duration;
    data.sampleRate = wavSpec.freq;

    loadedBuffers[path] = data;
    return data;
}

shared_ptr<SoundInstance> SoundManager::GetSoundFromPath(string path)
{
    auto buffer = LoadOrGetSoundFileBuffer(path);
    if (!buffer.context) return nullptr;

    ALuint source;
    //alGenSources(1, &source);
    //alSourcei(source, AL_BUFFER, buffer);

    return make_shared<SoundInstance>(buffer);
}

void SoundManager::CleanAllData()
{

    for (auto& kv : loadedBuffers) {
        alDeleteBuffers(1, &kv.second.buffer);
    }
    loadedBuffers.clear();

    for (auto b : loadedBanks)
    {
        if (b.second == nullptr) continue;

        b.second->unloadSampleData();
        b.second->unload();

    }

    loadedBanks.clear();

}


FMOD::Studio::Bank* SoundManager::LoadBankFromPath(
    const std::string& bankPath,
    bool               loadSampleData)
{
    if (!studioSystem)
    {
        Logger::Error("[FMOD] LoadBankFromPath: studioSystem is null");
        return nullptr;
    }

    // 1) cache check
    auto it = loadedBanks.find(bankPath);
    if (it != loadedBanks.end())
        return it->second;

    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result;

    // 2) Try loading directly from disk first
    result = studioSystem->loadBankFile(
        bankPath.c_str(),
        FMOD_STUDIO_LOAD_BANK_NORMAL,
        &bank
    );

    if (result == FMOD_OK && bank)
    {
        Logger::Info("[FMOD] Bank loaded from disk: %s", bankPath.c_str());
    }
    else
    {
        Logger::Warning("[FMOD] loadBankFile failed for '%s' (%d): %s",
            bankPath.c_str(), result, FMOD_ErrorString(result));

        // 3) Fallback: read file manually into memory
        std::vector<uint8_t> bankData;
        try {
            bankData = FileSystemEngine::ReadFileBinary(bankPath);
        }
        catch (const std::exception& e) {
            Logger::Error("[FMOD] Failed to read bank '%s': %s",
                bankPath.c_str(), e.what());
            loadedBanks[bankPath] = nullptr;
            return nullptr;
        }


        result = studioSystem->loadBankMemory(
            reinterpret_cast<const char*>(bankData.data()),
            static_cast<int>(bankData.size()),
            FMOD_STUDIO_LOAD_MEMORY, // or MEMORY_POINT (see note below)
            FMOD_STUDIO_LOAD_BANK_NORMAL,
            &bank
        );

        if (result != FMOD_OK || !bank)
        {
            Logger::Error("[FMOD] Failed to load bank '%s' from memory (%d): %s",
                bankPath.c_str(), result, FMOD_ErrorString(result));
            loadedBanks[bankPath] = nullptr;
            return nullptr;
        }

        Logger::Info("[FMOD] Bank loaded from memory: %s", bankPath.c_str());
    }

    // 4) optionally load sample data
    if (loadSampleData)
    {
        result = bank->loadSampleData();
        if (result != FMOD_OK)
            Logger::Warning("[FMOD] Warning: sample data failed for '%s' (%d): %s",
                bankPath.c_str(), result, FMOD_ErrorString(result));
        else
            Logger::Info("[FMOD] Sample data loaded for: %s", bankPath.c_str());
    }

    // 5) cache & return
    loadedBanks[bankPath] = bank;
    return bank;
}