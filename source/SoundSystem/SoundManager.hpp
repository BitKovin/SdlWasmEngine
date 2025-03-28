#ifndef SOUNDMANAGER_HPP
#define SOUNDMANAGER_HPP

#include <AL/al.h>
#include <AL/alc.h>

#include "../Logger.hpp"

#include "SoundInstance.hpp"
#include <SDL2/SDL_audio.h>
#include <unordered_map>

using namespace std;

class SoundManager
{

private:

	static ALCdevice* device;
	static ALCcontext* context;

	static std::unordered_map<std::string, ALuint> loadedBuffers;

public:

	static void Initialize()
	{
		device = alcOpenDevice(0);
		if (!device) {
			Logger::Log("Failed to open OpenAL device.\n");
			return;
		}
		context = alcCreateContext(device, 0);
		alcMakeContextCurrent(context);
	}

	static void Close()
	{
		// Cleanup
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	static ALuint LoadOrGetSoundFileBuffer(string path)
	{

		auto foundBuffer = loadedBuffers.find(path);

		if (foundBuffer != loadedBuffers.end())
		{
			return loadedBuffers[path];
		}

		SDL_AudioSpec wavSpec;
		Uint32 wavLength;
		Uint8* wavBuffer;

		if (SDL_LoadWAV(path.c_str(), &wavSpec, &wavBuffer, &wavLength) == NULL) {
			printf("Failed to load WAV file: %s\n", SDL_GetError());
			return 0;
		}

		ALuint buffer;

		// Transfer data to OpenAL
		alGenBuffers(1, &buffer);
		alBufferData(buffer, AL_FORMAT_STEREO16, wavBuffer, wavLength, wavSpec.freq);
		SDL_FreeWAV(wavBuffer); // Free after passing to OpenAL

		loadedBuffers[path] = buffer;
		return buffer;
	}

	static SoundInstance GetSoundFromPath(string path)
	{

		ALuint buffer = LoadOrGetSoundFileBuffer(path);

		ALuint source;

		// Set up OpenAL source
		alGenSources(1, &source);
		alSourcei(source, AL_BUFFER, buffer);

		SoundInstance soundInstance = SoundInstance();

		soundInstance.SetSoundRef(source);

		return soundInstance;

	}


};

std::unordered_map<std::string, ALuint> SoundManager::loadedBuffers;
ALCdevice* SoundManager::device = nullptr;
ALCcontext* SoundManager::context = nullptr;

#endif // !SOUNDMANAGER_HPP
