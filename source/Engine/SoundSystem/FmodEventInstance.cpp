#include "FmodEventInstance.h"
#include "SoundManager.hpp"
#include "../EngineMain.h"

// Static callback implementation
FMOD_RESULT FmodEventInstance::EventCallback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD_STUDIO_EVENTINSTANCE* event,
    void* parameters)
{
    FmodEventInstance* instance = nullptr;
    FMOD_Studio_EventInstance_GetUserData(event, (void**)&instance);

    if (!instance) return FMOD_OK;

    switch (type)
    {
    case FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND:
    {
        auto* props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameters;
        std::string soundName(props->name);

        // Try to get from programmer sounds map
        FMOD::Sound* sound = instance->GetProgrammerSound(soundName);
        if (sound)
        {
            props->sound = (FMOD_SOUND*)sound;
        }
        // Try default programmer sound
        else if (instance->defaultProgrammerSound)
        {
            props->sound = (FMOD_SOUND*)instance->defaultProgrammerSound;
        }
        // Try loading from sound table
        else if (!instance->soundTableKey.empty())
        {
            FMOD_STUDIO_SOUND_INFO info;
            FMOD::Sound* loadedSound = instance->GetSoundByName(instance->soundTableKey, &info);
            if (loadedSound)
            {
                props->sound = (FMOD_SOUND*)loadedSound;
                props->subsoundIndex = info.subsoundindex;

                // Mark this sound as owned by us
                std::lock_guard<std::mutex> lock(instance->ownedSoundsMutex);
                instance->ownedSounds.insert(loadedSound);
            }
        }
        break;
    }
    case FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND:
    {
        auto* props = (FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*)parameters;
        FMOD::Sound* sound = (FMOD::Sound*)props->sound;

        // Check if we own this sound
        std::lock_guard<std::mutex> lock(instance->ownedSoundsMutex);
        auto it = instance->ownedSounds.find(sound);
        if (it != instance->ownedSounds.end()) {
            (*it)->release();
            instance->ownedSounds.erase(it);
        }
        break;
    }
    }

    return FMOD_OK;
}

FmodEventInstance::FmodEventInstance(FMOD::Studio::EventInstance* instance)
    : eventInstance(instance)
{
    if (eventInstance)
    {
        eventInstance->setUserData(this);
        eventInstance->setCallback(FmodEventInstance::EventCallback,
            FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND |
            FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND);
    }
}

FmodEventInstance::~FmodEventInstance()
{
    if (eventInstance)
    {
        // Check if the event is already stopped or stopping. 
        // If not, allow it to fade out naturally rather than cutting it off.
        FMOD_STUDIO_PLAYBACK_STATE state;
        eventInstance->getPlaybackState(&state);

        if (state != FMOD_STUDIO_PLAYBACK_STOPPED && state != FMOD_STUDIO_PLAYBACK_STOPPING)
        {
            eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        }

        eventInstance->setUserData(nullptr);
        eventInstance->setCallback(nullptr);

        // This tells FMOD's internal system to free the memory ONLY AFTER 
        // the event fully finishes its AHDSR release fade.
        eventInstance->release();

        eventInstance = nullptr;
    }

    // Release any remaining owned sounds (for programmer sounds)
    std::lock_guard<std::mutex> lock(ownedSoundsMutex);
    for (auto* sound : ownedSounds) {
        sound->release();
    }
    ownedSounds.clear();
}

void FmodEventInstance::Play()
{
    if (!eventInstance) return;

    FMOD_STUDIO_PLAYBACK_STATE state;
    eventInstance->getPlaybackState(&state);

    if (state == FMOD_STUDIO_PLAYBACK_PLAYING) {
        eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
    }

    eventInstance->start();
    eventInstance->setVolume(0); // Start muted until first update
}

void FmodEventInstance::Stop()
{
    if (eventInstance) {
        eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
    }
}

void FmodEventInstance::StopImmediate()
{
    if (eventInstance) {
        eventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    }
}

void FmodEventInstance::Update(float deltaTime)
{
    if (!eventInstance) return;

    // Update base properties
    eventInstance->setVolume(GetFinalVolume());
    eventInstance->setPitch(GetPitchScale() * Pitch);
    eventInstance->setPaused(Paused || IsGamePaused());

    // Apply 3D positioning if needed
    if (!Is2D) {
        Apply3D();
    }
}

void FmodEventInstance::Apply3D()
{
    FMOD_3D_ATTRIBUTES attributes = {};
    attributes.position = { Position.x, Position.y, Position.z };
    attributes.velocity = { Velocity.x, Velocity.y, Velocity.z };
    attributes.forward = { Direction.x, Direction.y, Direction.z };
    attributes.up = { 0.0f, 1.0f, 0.0f };

    eventInstance->set3DAttributes(&attributes);
}

void FmodEventInstance::SetParameter(const std::string& name, float value)
{
    if (eventInstance) {
        eventInstance->setParameterByName(name.c_str(), value);
    }
}

void FmodEventInstance::SetProgrammerSound(const std::string& name, FMOD::Sound* sound)
{
    programmerSounds[name] = sound;
}

FMOD::Sound* FmodEventInstance::GetProgrammerSound(const std::string& name)
{
    auto it = programmerSounds.find(name);
    return (it != programmerSounds.end()) ? it->second : nullptr;
}

FMOD::Sound* FmodEventInstance::GetSoundByName(const std::string& name, FMOD_STUDIO_SOUND_INFO* outInfo)
{
    if (!SoundManager::studioSystem || !SoundManager::coreSystem)
        return nullptr;

    FMOD_RESULT result = SoundManager::studioSystem->getSoundInfo(name.c_str(), outInfo);
    if (result != FMOD_OK)
        return nullptr;

    FMOD::Sound* sound = nullptr;
    result = SoundManager::coreSystem->createSound(
        outInfo->name_or_data,
        outInfo->mode,
        &outInfo->exinfo,
        &sound
    );

    return (result == FMOD_OK) ? sound : nullptr;
}

// Factory methods
std::shared_ptr<FmodEventInstance> FmodEventInstance::Create(const std::string& eventPath)
{
    if (!SoundManager::studioSystem)
        return nullptr;

    FMOD::Studio::EventDescription* eventDesc = nullptr;
    SoundManager::studioSystem->getEvent(eventPath.c_str(), &eventDesc);

    if (!eventDesc)
        return nullptr;

    eventDesc->loadSampleData(); // Synchronous load

    FMOD::Studio::EventInstance* instance = nullptr;
    eventDesc->createInstance(&instance);

    return instance ? std::make_shared<FmodEventInstance>(instance) : nullptr;
}

std::shared_ptr<FmodEventInstance> FmodEventInstance::CreateFromId(const std::string& guid)
{
    if (!SoundManager::studioSystem)
        return nullptr;

    FMOD_GUID fmodGuid;
    FMOD_Studio_ParseID(guid.c_str(), &fmodGuid);

    FMOD::Studio::EventDescription* eventDesc = nullptr;
    SoundManager::studioSystem->getEventByID(&fmodGuid, &eventDesc);

    if (!eventDesc)
        return nullptr;

    FMOD::Studio::EventInstance* instance = nullptr;
    eventDesc->createInstance(&instance);

    return instance ? std::make_shared<FmodEventInstance>(instance) : nullptr;
}

bool FmodEventInstance::IsGamePaused() const
{
    if (IsUISound) return false;
    return EngineMain::MainInstance->Paused;
}

float FmodEventInstance::GetPitchScale() const
{
    if (IsUISound)
        return 1.0f;

    return Time::GetSoundFinalTimeScale();
}

float FmodEventInstance::GetFinalVolume() const
{
    return Volume * SoundManager::GlobalVolume * SoundManager::GetVolumeForSoundType(Type);
}