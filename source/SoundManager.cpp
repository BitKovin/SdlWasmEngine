#include "SoundSystem/SoundManager.hpp"

std::unordered_map<std::string, ALuint> SoundManager::loadedBuffers;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::context = nullptr;