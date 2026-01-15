#include <Entities/SoundPlayer.h>
#include <SpatialSound/SpatialSoundManager.h>
#include <Input.h>
#include <Camera.h>
#include <SoundSystem/SoundInstance.hpp>

class TestSpatialSoundPlayer : public SoundPlayer
{
public:
	
	void Start() override 
	{
		SetSound(SoundManager::GetSoundFromPath("GameData/sounds/mew.wav"));
        Play();
		Sound->Loop = true;
        MaxDistance = 70;
	}

	void Update() override
	{



		auto result = SpatialSoundManager::ComputeAudioSource(Position, MaxDistance);
		SpatialSoundManager::listener = Camera::position;

        vec3 entPos = Position;
        float dist = distance(Position, SpatialSoundManager::listener);

        Position = SpatialSoundManager::listener - result.reverb.bounceDirection * dist;

        SoundPlayer::LateUpdate();

        Position = entPos;

		std::string debugOut = "occlusion gain:" + to_string(result.occlusionGain);
		debugOut += ("\naverage distance:" + to_string(result.reverb.averageDistance));
		debugOut += ("\naverage energy:" + to_string(result.reverb.averageEnergy));
        debugOut += ("\naverage bounce:" + to_string(result.reverb.averageBounces));
        debugOut += ("\nairspace:" + to_string(result.reverb.airspace));

        DebugDraw::Line(Camera::position - vec3(0, 1, 0), Camera::position - vec3(0, 1, 0) - result.reverb.bounceDirection, 0.002f);

		Logger::Log(debugOut);

		if (Input::GetAction("slotMelee")->Holding())
		{
			Position = Camera::position;
		}


        SoundInstance* instance = dynamic_cast<SoundInstance*>(Sound.get());
        if (instance)
        {
            const float speedOfSound = 343.0f;
            const float energyThresholdDB = 30.0f; // Since 0.001 is approx -30 dB drop
            const float targetDB = 60.0f; // Standard RT60 is -60 dB

            // Occlusion: Apply low-pass filter for muffling effect
            bool EnableFilter = (result.occlusionGain < 0.99f); // Enable if any occlusion
            float LowPassGain = result.occlusionGain; // Overall gain attenuation from occlusion
            float LowPassGainHF = std::pow(result.occlusionGain, 1.5f); // Stronger attenuation on high frequencies for realistic muffling

            // Reverb: Enable if there's enclosed space (airspace indicates fraction of reverberant paths)
            bool EnableReverb = (result.reverb.airspace > 0.05f); // Small threshold to avoid in fully open spaces
            float ReverbDensity = 0.5f;
            float ReverbGain = 0.5f;
            float ReverbGainHF = 0.3f;
            float ReverbDecayTime = 1.0f;

            if (EnableReverb) {
                // Approximate RT60: Scale the time to low energy threshold to full -60 dB
                float timeToThreshold = result.reverb.averageDistance / speedOfSound;
                ReverbDecayTime = (targetDB / energyThresholdDB) * timeToThreshold; // Approx 2x for -30 to -60 dB

                // Clamp to sane values (e.g., 0.1s to 20s)
                ReverbDecayTime = std::clamp(ReverbDecayTime, 0.1f, 20.0f);

                // Reverb gain based on average remaining energy, scaled by airspace (reverberant field strength)
                ReverbGain = result.reverb.averageEnergy * result.reverb.airspace;
                ReverbGain = std::clamp(ReverbGain, 0.0f, 1.0f);

                // HF gain: Assume higher frequencies decay faster, scale down
                ReverbGainHF = ReverbGain * 0.7f; // Arbitrary factor for HF absorption

                // Density: Higher with more bounces (more diffuse in smaller/more complex spaces)
                ReverbDensity = result.reverb.averageBounces / 50.0f; // Normalize by maxBounces=50
                ReverbDensity = std::clamp(ReverbDensity, 0.0f, 1.0f);
            }

            // Echo: Enable for larger spaces with fewer bounces (like halls/caves for distinct echoes)
            bool EnableEcho = EnableReverb && (ReverbDecayTime > 1.5f);
            float EchoDelay = 0.1f;
            float EchoLRDelay = 0.1f;
            float EchoDamping = 0.5f;
            float EchoFeedback = 0.5f;
            float EchoSpread = 0.5f;

            if (EnableEcho) {
                // Delay based on average path time (divided for approximate first reflection)
                EchoDelay = (result.reverb.averageDistance / speedOfSound) / 3.0f; // Arbitrary divisor for echo delay
                EchoDelay = std::clamp(EchoDelay, 0.05f, 0.5f) * result.reverb.averageBounces / 4;

                EchoLRDelay = EchoDelay * 0.5f; // Slight left-right offset
                EchoFeedback = result.reverb.averageEnergy * 1.6f; // Feedback based on energy
                EchoDamping = 0.6f; // Moderate damping
                EchoSpread = 0.8f; // Some spread
            }

            // Apply to instance (assuming SoundInstance has these as members or setters)
            instance->EnableFilter = EnableFilter;
            instance->LowPassGain = LowPassGain;
            instance->LowPassGainHF = LowPassGainHF;
            instance->EnableEcho = EnableEcho;
            instance->EchoDelay = EchoDelay;
            instance->EchoLRDelay = EchoLRDelay;
            instance->EchoDamping = EchoDamping;
            instance->EchoFeedback = EchoFeedback;
            instance->EchoSpread = EchoSpread;
            instance->EnableReverb = EnableReverb;
            instance->ReverbDensity = ReverbDensity;
            instance->ReverbGain = ReverbGain;
            instance->ReverbGainHF = ReverbGainHF;
            instance->ReverbDecayTime = ReverbDecayTime;
        }

	}

    void LateUpdate() override{}

private:

};

REGISTER_ENTITY(TestSpatialSoundPlayer, "TestSpatialSoundPlayer")
