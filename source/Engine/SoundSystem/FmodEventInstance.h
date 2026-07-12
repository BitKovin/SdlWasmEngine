#pragma once

#include "fmod_include.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <set>
#include "../glm.h"
#include "SoundInstance.hpp"

class FmodEventInstance : public SoundInstanceBase
{
public:
    FmodEventInstance(FMOD::Studio::EventInstance* instance);
    ~FmodEventInstance();

    // SoundInstanceBase overrides
    void Play() override;
    void Stop() override;
    void StopImmediate();
    void Update(float deltaTime) override;

    // Custom methods
    void SetParameter(const std::string& name, float value);
    void SetProgrammerSound(const std::string& name, FMOD::Sound* sound);
    FMOD::Sound* GetProgrammerSound(const std::string& name);
    void SetSoundTableKey(const std::string& key) { soundTableKey = key; }
    void SetDefaultProgrammerSound(FMOD::Sound* sound) { defaultProgrammerSound = sound; }

    static std::shared_ptr<FmodEventInstance> Create(const std::string& eventPath);
    static std::shared_ptr<FmodEventInstance> CreateFromId(const std::string& guid);

protected:
    bool IsGamePaused() const;
    float GetPitchScale() const;
    float GetFinalVolume() const;

private:
    FMOD::Studio::EventInstance* eventInstance = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> programmerSounds;
    FMOD::Sound* defaultProgrammerSound = nullptr;
    std::string soundTableKey;
    std::set<FMOD::Sound*> ownedSounds; // For sounds we created and own
    std::mutex ownedSoundsMutex;

    void Apply3D();
    FMOD::Sound* GetSoundByName(const std::string& name, FMOD_STUDIO_SOUND_INFO* outInfo);

    // Callback handling
    static FMOD_RESULT EventCallback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD_STUDIO_EVENTINSTANCE* event,
        void* parameters);
};