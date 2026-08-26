#include <algorithm>
#include <Particle/ParticleSystem.hpp>
#include <Particle/RibbonEmitter.h>

// Multiplier applied to every time-based value that drives this particle/
// emitter's lifetime (per-particle deathTime, fade timing offset, the
// emitterTime windows used for transparency mapping and cutoff). 1.0f =
// default duration; >1.0f makes the smoke last longer, <1.0f shortens it.
// NOTE: keep this in sync with WEAPONSMOKE_DURATION_FACTOR in
// WeaponFirearm.h - that copy drives the firearm's trail recycle/stop-delay
// logic, and the two need to agree or the trail will be respawned or torn
// down out of step with this emitter's actual fade curve.
constexpr float WEAPONSMOKE_DURATION_FACTOR = 0.4f;

// --- Velocity-based trail realism ---
// The muzzle position is reported every frame (WeaponFirearm just sets
// Position); everything about how the trail reacts to that motion lives
// here. Real smoke has inertia and drag - it doesn't teleport with the gun -
// so instead of snapping the trail's leading edge straight to the muzzle
// every frame (which turns fast weapon motion into one huge rigid ribbon),
// the tip chases the muzzle at a capped speed, and fresh puffs only briefly
// inherit a bit of the weapon's motion before settling into their own drift.
constexpr float WEAPONSMOKE_MAX_FOLLOW_SPEED = 7.5f;            // m/s the trail's leading edge can chase the muzzle
constexpr float WEAPONSMOKE_VELOCITY_SMOOTHING_RATE = 15.0f;    // 1/s, smooths measured muzzle velocity so a single-frame stutter/teleport doesn't spike the trail
constexpr float WEAPONSMOKE_VELOCITY_INHERIT_FRACTION = 0.35f;  // fraction of the (smoothed) muzzle velocity a freshly spawned puff inherits
constexpr float WEAPONSMOKE_VELOCITY_INHERIT_TIME = 0.35f;      // seconds (scaled by WEAPONSMOKE_DURATION_FACTOR) before a puff "lets go" of that inherited motion and just drifts

class WeaponSmokeEmitter : public RibbonEmitter
{
public:


	WeaponSmokeEmitter() : RibbonEmitter()
	{
		texture = "GameData/textures/particles/trail.png";
		SpawnRate = 20;
		Emitting = true;
		InitialSpawnCount = 1;
		SimpleRibbon = false;
		IsViewmodel = true;
	}

	float easeOutCubic(float val)
	{
		return 1.0f - powf(1.0f - val, 2.0f);
	}

	Particle UpdateParticle(Particle particle, float deltaTime) override
	{

		float deathTime = particle.deathTime - 0.1f * WEAPONSMOKE_DURATION_FACTOR;

		particle.Transparency = mix(0.1, 1.0, (deathTime - particle.lifeTime) / deathTime);
		particle.Size = mix(0.055f, 0.025f, (deathTime - particle.lifeTime) / deathTime);

		float t = particle.lifeTime / deathTime;
		float eased = easeOutCubic(t);

		float verticalSpeed = mix(0.8f, 0.25f, eased);

		// Fresh puffs still caught in the muzzle's turbulence carry a bit of
		// the weapon's own velocity with them; that influence fades out as
		// the puff ages and detaches, so it doesn't keep chasing the gun
		// forever - just gives it a believable initial "flung" drift.
		float velocityInfluence = std::clamp(1.0f - particle.lifeTime / (WEAPONSMOKE_VELOCITY_INHERIT_TIME * WEAPONSMOKE_DURATION_FACTOR), 0.0f, 1.0f);
		vec3 inheritedDrift = emitterVelocity * WEAPONSMOKE_VELOCITY_INHERIT_FRACTION * velocityInfluence;

		particle.position += (vec3(0, verticalSpeed, 0) + inheritedDrift) * Time::DeltaTimeF;

		return particle;
	}

	Particle GetNewParticle() override
	{
		Particle particle = ParticleEmitter::GetNewParticle();

		float systemTransparency = MathHelper::MapRange(emitterTime, 0.5f * WEAPONSMOKE_DURATION_FACTOR, 1.8f * WEAPONSMOKE_DURATION_FACTOR, 1, 0);

		// Spawn at the lagged trail tip (see Update()), not the raw muzzle
		// position - otherwise a brand new puff would appear exactly on the
		// muzzle and then get yanked back to the lagging tip a frame later.
		particle.position = trailTip;

		particle.Size = 0.03f;
		particle.Color = vec4(0.8, 0.8, 0.8, 0.2 * systemTransparency);

		particle.Transparency = 0.8;
		particle.deathTime = 1.4f * WEAPONSMOKE_DURATION_FACTOR;

		return particle;
	}

	void Update(float deltaTime)
	{

		// --- Work out how fast the muzzle is currently moving ---
		if (!trailTipInitialized)
		{
			lastEmitterPosition = Position;
			trailTip = Position;
			trailTipInitialized = true;
		}

		vec3 rawVelocity = deltaTime > 0.0001f ? (Position - lastEmitterPosition) / deltaTime : vec3(0.0f);
		float smoothing = std::clamp(deltaTime * WEAPONSMOKE_VELOCITY_SMOOTHING_RATE, 0.0f, 1.0f);
		emitterVelocity = mix(emitterVelocity, rawVelocity, smoothing);
		lastEmitterPosition = Position;

		// --- Let the trail's leading edge chase the muzzle, but only up to
		// a fixed physical speed. Real smoke has drag/inertia and can't
		// teleport with the source, so fast weapon motion makes the trail
		// lag and thin out instead of stretching into one huge rigid ribbon.
		vec3 toMuzzle = Position - trailTip;
		float distToMuzzle = length(toMuzzle);
		float maxStep = WEAPONSMOKE_MAX_FOLLOW_SPEED * deltaTime;

		if (distToMuzzle > maxStep)
			trailTip += (toMuzzle / distToMuzzle) * maxStep;
		else
			trailTip = Position;

		RibbonEmitter::Update(deltaTime);

		if (emitterTime > 1.8f * WEAPONSMOKE_DURATION_FACTOR) { Emitting = false; }

		if (emitterTime < 1.85f * WEAPONSMOKE_DURATION_FACTOR)
		{
			if (Particles.size() > 0)
			{
				Particles[Particles.size() - 1].position = trailTip;
			}
		}


	}

private:

	// Velocity-aware trailing state (see Update()/GetNewParticle()/UpdateParticle())
	vec3 lastEmitterPosition = vec3(0.0f);
	vec3 emitterVelocity = vec3(0.0f);
	vec3 trailTip = vec3(0.0f);
	bool trailTipInitialized = false;

};

class WeaponSmoke : public ParticleSystem
{
public:
	WeaponSmoke()
	{
		emitters.push_back(new WeaponSmokeEmitter());
	}


private:

};

REGISTER_ENTITY(WeaponSmoke, "weapon_smoke")