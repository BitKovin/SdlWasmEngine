#pragma once

// What pass a draw command belongs to - replaces the old per-IDrawMesh "Transparent" bool. Never set directly, derived from actual data - see StaticMeshColorEmissiveMaterial::EffectiveSurfaceType.
enum class SurfaceType
{
	NONE,        // not yet determined - treated as Opaque if it leaks through
	Opaque,
	Masked,      // alpha-tested cutout, still depth-writes like Opaque
	Transparent  // alpha-blended, no depth write, sorted back-to-front
};
