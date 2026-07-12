#pragma once

#include <stdbool.h>
#include <math.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VAMatrix VAMatrix;
typedef struct VAVector VAVector;
typedef struct VAEAXReverb VAEAXReverb;
typedef struct VALowPassFilter VALowPassFilter;
typedef struct VAVisualisationData VAVisualisationData;
typedef struct VAProcessedReverb VAProcessedReverb;

typedef int VAResult;

#define VA_SUCCESS          0
#define VA_INVALID_VALUE    1
#define VA_OUT_OF_RANGE     2
#define VA_ALREADY_EXISTS   3
#define VA_FEATURE_DISABLED 4
#define VA_ERROR_IN_USE     5

#ifdef _WIN32
    #define EXPORT_API __declspec(dllexport)
    #define HIDDEN_API
    #define UNUSED
#else
    #define EXPORT_API __attribute__((visibility("default")))
    #define HIDDEN_API __attribute__((visibility("hidden")))
    #define UNUSED __attribute__((unused))
#endif

// This struct is column-major
typedef struct VAMatrix
{
    float m11;
    float m12;
    float m13;
    float m14;
    float m21;
    float m22;
    float m23;
    float m24;
    float m31;
    float m32;
    float m33;
    float m34;
    float m41;
    float m42;
    float m43;
    float m44;
} VAMatrix;

extern const VAMatrix VA_MATRIX_IDENTITY;
extern const VAMatrix VA_MATRIX_EMPTY;

// Initializes a matrix with all 16 elements in column-major order (each group of 4 is one column)
EXPORT_API VAMatrix vaMatrixCreate(float m11, float m12, float m13, float m14,
                                   float m21, float m22, float m23, float m24,
                                   float m31, float m32, float m33, float m34,
                                   float m41, float m42, float m43, float m44);

// Returns the inverse of the matrix. Asserts if the determinant is zero (no inverse exists)
EXPORT_API VAMatrix vaMatrixInverse(const VAMatrix* mat);
// Creates a translation matrix
EXPORT_API VAMatrix vaMatrixCreateTranslation(float x, float y, float z);
// Creates a rotation matrix around the X axis. rads is the angle in radians
EXPORT_API VAMatrix vaMatrixCreateRotationX(float rads);
// Creates a rotation matrix around the Y axis. rads is the angle in radians
EXPORT_API VAMatrix vaMatrixCreateRotationY(float rads);
// Creates a rotation matrix around the Z axis. rads is the angle in radians
EXPORT_API VAMatrix vaMatrixCreateRotationZ(float rads);
// Creates a scale matrix. Note scale matrices can only be set on mesh primitives and not any other primitive
EXPORT_API VAMatrix vaMatrixCreateScale(float scaleX, float scaleY, float scaleZ);
// Multiplies two matrices together
EXPORT_API VAMatrix vaMatrixMultiply(const VAMatrix* a, const VAMatrix* b);
// Returns true if all elements of a are exactly equal to those in b
EXPORT_API bool vaMatrixIsEqual(const VAMatrix* a, const VAMatrix* b);

typedef struct VAVector
{
    float x;
    float y;
    float z;
} VAVector;

#ifndef VECTOR_CONSTANTS
#define VECTOR_CONSTANTS
// A vector with all components set to zero
static const VAVector VECTOR_ZERO = { 0, 0, 0 };
// A vector with all components set to one
static const VAVector VECTOR_ONE = { 1, 1, 1 };
// A unit vector pointing upward (0, 1, 0)
static const VAVector VECTOR_UP = { 0, 1, 0 };
// A vector with all components set to FLT_MIN
static const VAVector VECTOR_MIN = { FLT_MIN, FLT_MIN, FLT_MIN };
// A vector with all components set to FLT_MAX
static const VAVector VECTOR_MAX = { FLT_MAX, FLT_MAX, FLT_MAX };
#endif // VECTOR_CONSTANTS

// Creates a vector with the specified components
static inline VAVector vaVectorCreate(float x, float y, float z)
{
#ifdef __cplusplus
    VAVector v = { x, y, z }; return v;
#else
    return (VAVector){ x, y, z };
#endif
}

// Creates a vector with all components set to value
static inline VAVector vaVectorCreateUniform(float value)
{
#ifdef __cplusplus
    VAVector v = { value, value, value }; return v;
#else
    return (VAVector){ value, value, value };
#endif
}

// Returns the length of v
static inline float vaVectorMagnitude(VAVector v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Returns the component-wise sum of a and b
static inline VAVector vaVectorAdd(VAVector a, VAVector b)
{
    return vaVectorCreate(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Returns the component-wise difference of a and b
static inline VAVector vaVectorSubtract(VAVector a, VAVector b)
{
    return vaVectorCreate(a.x - b.x, a.y - b.y, a.z - b.z);
}

// Scales a by scalar
static inline VAVector vaVectorMultiplyScalar(VAVector a, float scalar)
{
    return vaVectorCreate(a.x * scalar, a.y * scalar, a.z * scalar);
}

// Divides a by scalar
static inline VAVector vaVectorDivideScalar(VAVector a, float scalar)
{
    return vaVectorCreate(a.x / scalar, a.y / scalar, a.z / scalar);
}

// Returns true if all components of a are greater than or equal to those of b
static inline bool vaVectorGreaterEqual(VAVector a, VAVector b)
{
    return a.x >= b.x && a.y >= b.y && a.z >= b.z;
}

// Returns true if all components of a are less than or equal to those of b
static inline bool vaVectorLessEqual(VAVector a, VAVector b)
{
    return a.x <= b.x && a.y <= b.y && a.z <= b.z;
}

// Returns true if any component of a is less than the corresponding component of b
static inline bool vaVectorLessAny(VAVector a, VAVector b)
{
    return a.x < b.x || a.y < b.y || a.z < b.z;
}

// Returns true if any component of a is greater than the corresponding component of b
static inline bool vaVectorGreaterAny(VAVector a, VAVector b)
{
    return a.x > b.x || a.y > b.y || a.z > b.z;
}

// Returns true if all components of a equal those of b
static inline bool vaVectorEqual(VAVector a, VAVector b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

// Returns the component-wise minimum of a and b
static inline VAVector vaVectorMin(VAVector a, VAVector b)
{
    return vaVectorCreate(
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z
    );
}

// Returns the component-wise maximum of a and b
static inline VAVector vaVectorMax(VAVector a, VAVector b)
{
    return vaVectorCreate(
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z
    );
}

// Gets a component by index (0=x, 1=y, 2=z)
static inline float vaVectorGetComponent(VAVector v, int index)
{
    if (index == 0) return v.x;
    if (index == 1) return v.y;
    return v.z;
}

// Returns the dot product of a and b
static inline float vaVectorDot(VAVector a, VAVector b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Returns the cross product of a and b
static inline VAVector vaVectorCross(VAVector a, VAVector b)
{
    float x = a.y * b.z - a.z * b.y;
    float y = a.z * b.x - a.x * b.z;
    float z = a.x * b.y - a.y * b.x;

    return vaVectorCreate(x, y, z);
}

// Returns the negation of a
static inline VAVector vaVectorNegate(VAVector a)
{
    return vaVectorCreate(-a.x, -a.y, -a.z);
}

// Returns a normalized copy of v
static inline VAVector vaVectorNormalize(VAVector v)
{
    float invMag = 1.0f / vaVectorMagnitude(v);
    return vaVectorCreate(v.x * invMag, v.y * invMag, v.z * invMag);
}

// Creates a direction vector from pitch and yaw angles in radians
static inline VAVector vaVectorFromPitchYaw(float pitch, float yaw)
{
    float cosPitch = cosf(pitch);
    return vaVectorCreate(-cosPitch * sinf(yaw), sinf(pitch), -cosPitch * cosf(yaw));
}

// Transforms a point by a matrix, including translation (w=1)
static inline VAVector vaVectorMultiplyMatrix(VAVector a, const VAMatrix* b)
{
    float x = a.x * b->m11 + a.y * b->m21 + a.z * b->m31 + b->m41;
    float y = a.x * b->m12 + a.y * b->m22 + a.z * b->m32 + b->m42;
    float z = a.x * b->m13 + a.y * b->m23 + a.z * b->m33 + b->m43;

    return vaVectorCreate(x, y, z);
}

// Transforms a normal vector by a matrix, ignoring translation, and normalizes the result
static inline VAVector vaVectorTransformNormal(VAVector a, const VAMatrix* b)
{
    float x = a.x * b->m11 + a.y * b->m21 + a.z * b->m31;
    float y = a.x * b->m12 + a.y * b->m22 + a.z * b->m32;
    float z = a.x * b->m13 + a.y * b->m23 + a.z * b->m33;

    return vaVectorNormalize(vaVectorCreate(x, y, z));
}

// Transforms a normal by the transpose of the given inverse matrix (inverse-transpose of the forward transform), supporting non-uniform scale
static inline VAVector vaVectorTransformNormalByInverseTranspose(VAVector a, const VAMatrix* inverseMatrix)
{
    float x = a.x * inverseMatrix->m11 + a.y * inverseMatrix->m12 + a.z * inverseMatrix->m13;
    float y = a.x * inverseMatrix->m21 + a.y * inverseMatrix->m22 + a.z * inverseMatrix->m23;
    float z = a.x * inverseMatrix->m31 + a.y * inverseMatrix->m32 + a.z * inverseMatrix->m33;

    return vaVectorNormalize(vaVectorCreate(x, y, z));
}

// Returns true if any component of vec is NaN or infinity
static inline bool vaVectorIsNanOrInfinity(VAVector vec)
{
    return isnan(vec.x) || isinf(vec.x) || isnan(vec.y) || isinf(vec.y) || isnan(vec.z) || isinf(vec.z);
}

// Reflects direction about normal
static inline VAVector vaVectorReflect(VAVector direction, VAVector normal)
{
    float dotProduct = vaVectorDot(direction, normal);
    VAVector scaledNormal = vaVectorMultiplyScalar(normal, 2.0f * dotProduct);
    return vaVectorSubtract(direction, scaledNormal);
}

typedef struct VAEmitter VAEmitter;

/* Computed EAX reverb parameters */
typedef struct VAEAXReverb
{
    float outsidePercent;
    float returnedPercent;
    float materialAbsorptionLF;
    float materialAbsorptionHF;
    float materialRoughness;

    VAEmitter** relativeEmitters;   /* Emitter keys for relativeDirections/relativeGains */
    VAVector* relativeDirections;   /* Direction that reverb should be heard from, relative to each emitter with hasRelativeReverb=true. Access via vaEaxReverbGetRelativeDirection */
    float* relativeGains;           /* Gain for this reverb effect, relative to each emitter with hasRelativeReverb=true. Access via vaEaxReverbGetRelativeGain */
    int relativeCount;
    int relativeCapacity;

    float reflectionsDelay;         /* Delay before early reflections are heard, in seconds (0–0.3) */
    float density;                  /* Modal density of the late reverberation (0–1) */
    float diffusion;                /* Echo diffusion of the late reverberation (0–1) */
    float gainLF;                   /* Low-frequency gain of the reverb (0–1) */
    float gainHF;                   /* High-frequency gain of the reverb (0–1) */
    float gain;                     /* Overall linear gain of the reverb (0–1) */
    float decayTime;                /* Reverberation decay time at mid frequencies, in seconds (0.1–20) */
    float decayLFRatio;             /* Ratio of low-frequency decay time to mid-frequency decay time (0.1–2) */
    float decayHFRatio;             /* Ratio of high-frequency decay time to mid-frequency decay time (0.1–2) */
    float reflectionsGain;          /* Linear gain of early reflections (0–3.16) */
    float lateReverbGain;           /* Linear gain of late reverberation (0–10) */
    float lateReverbDelay;          /* Delay of late reverberation relative to early reflections, in seconds (0–0.1) */
    float echoTime;                 /* Cycling time of the echo effect, in seconds (0.075–0.25) */
    float echoDepth;                /* Amplitude of the echo effect (0–1) */
    float modulationTime;           /* Cycling time of the modulation effect, in seconds (0.04–4) */
    float modulationDepth;          /* Amplitude of the modulation effect (0–1) */
    float airAbsorptionGainHF;      /* Linear gain applied per meter of distance for high-frequency air absorption (0.892–1) */
    float hfReference;              /* Reference frequency for high-frequency decay ratio, in Hz (1000–20000) */
    float lfReference;              /* Reference frequency for low-frequency decay ratio, in Hz (20–1000) */
    float roomRolloffFactor;        /* Rolloff factor for reflected sound sources (0–10) */
    int decayHFLimit;               /* Whether to limit high-frequency decay time to the air absorption limit (0 or 1) */

    int isTempBackground;
} VAEAXReverb;

/* Returns NULL if no entry exists for this emitter. */
EXPORT_API VAVector* vaEaxReverbGetRelativeDirection(const VAEAXReverb* eax, const VAEmitter* emitter);

/* Returns NULL if no entry exists for this emitter. */
EXPORT_API float* vaEaxReverbGetRelativeGain(const VAEAXReverb* eax, const VAEmitter* emitter);

typedef float (*VAEmitterGainFormula)(bool lowFrequency, int occlusionRayCount, int permeationRayCount, int permeationBounceCount, float occlusionEnergy, float permeationEnergy);
typedef void (*VAEmitterOnRaytracingComplete)(void);
typedef void (*VAEmitterOnRaytracedByAnotherEmitter)(struct VAEmitter* source, struct VAEmitter* target);
typedef void (*VAEmitterVisualisationCallback)(VAVisualisationData* data, int count);

typedef void (*VALogCallback)(const char* message);

// LF and HF gain to apply to a low pass filter
typedef struct VALowPassFilter
{
    // Low-frequency gain in the range 0.0 to 1.0
    float gainLF;
    // High-frequency gain in the range 0.0 to 1.0
    float gainHF;
} VALowPassFilter;

typedef struct VAVisualisationData
{
    VAVector position;
    VAVector normal;
} VAVisualisationData;

/* A 3D position that casts rays and is discovered by other emitters. */

/* Creates a new emitter with default settings. */
EXPORT_API VAEmitter* vaEmitterCreate(void);
/* Frees all resources owned by this emitter. Returns VA_ERROR_IN_USE if the emitter is still added to a world. */
EXPORT_API VAResult vaEmitterDestroy(VAEmitter* emitter);

/* Returns true if target has been added to this emitter via vaEmitterAddTarget(). */
EXPORT_API bool vaEmitterHasTarget(VAEmitter* emitter, VAEmitter* target);
/* Returns true if raytracing has produced results for this target at least once. */
EXPORT_API bool vaEmitterHasRaytracedTarget(VAEmitter* emitter, VAEmitter* target);
/* Only safe to call when vaEmitterHasRaytracedTarget() returns true. */
EXPORT_API VALowPassFilter* vaEmitterGetTargetFilter(VAEmitter* emitter, VAEmitter* target);

/* Returns VA_SUCCESS on success, VA_ALREADY_EXISTS if the emitter already has this target, VA_FEATURE_DISABLED if the emitter does not cast occlusion or permeation rays.
   An emitter must cast occlusion or permeation rays before adding targets. */
EXPORT_API VAResult vaEmitterAddTarget(VAEmitter* emitter, VAEmitter* target);
/* Removes a previously added target from this emitter. */
EXPORT_API void vaEmitterRemoveTarget(VAEmitter* emitter, VAEmitter* target);

/* Invalidates the ray cache; all rays will be re-cast on the next Update(). */
EXPORT_API void vaEmitterResetTrails(VAEmitter* emitter);

/* Number of reverb rays cast per frame. */
EXPORT_API void vaEmitterSetReverbRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per reverb ray. */
EXPORT_API void vaEmitterSetReverbBounceCount(VAEmitter* emitter, int value);
/* Number of occlusion rays cast per frame. */
EXPORT_API void vaEmitterSetOcclusionRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per occlusion ray. */
EXPORT_API void vaEmitterSetOcclusionBounceCount(VAEmitter* emitter, int value);
/* Number of permeation rays cast per frame. */
EXPORT_API void vaEmitterSetPermeationRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per permeation ray. */
EXPORT_API void vaEmitterSetPermeationBounceCount(VAEmitter* emitter, int value);
/* Number of ambient occlusion rays cast per frame. */
EXPORT_API void vaEmitterSetAmbientOcclusionRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per ambient occlusion ray. */
EXPORT_API void vaEmitterSetAmbientOcclusionBounceCount(VAEmitter* emitter, int value);
/* Number of ambient permeation rays cast per frame. */
EXPORT_API void vaEmitterSetAmbientPermeationRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per ambient permeation ray. */
EXPORT_API void vaEmitterSetAmbientPermeationBounceCount(VAEmitter* emitter, int value);

/* Energy threshold below which permeation rays are cancelled to prevent unnecessary traversal. */
EXPORT_API void vaEmitterSetMinimumPermeationEnergy(VAEmitter* emitter, float value);

/* Amount of returning energy required for reverb to be at maximum volume. Defaults to 0.2f.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if value is NaN or Infinity, VA_OUT_OF_RANGE if value is less than 0.0001f or greater than 1.0f. */
EXPORT_API VAResult vaEmitterSetReverbEnergyCap(VAEmitter* emitter, float value);

/* Rays are cast from these positions instead of the default position. Pass NULL to clear.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if positions is non-NULL but count is 0 or contains NaN/Infinity. */
EXPORT_API VAResult vaEmitterSetOverridePositions(VAEmitter* emitter, VAVector* positions, int count);
/* Rays are cast in these directions instead of the default spherical distribution. Pass NULL to clear.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if directions is non-NULL but count is 0 or contains non-normalisable vectors. */
EXPORT_API VAResult vaEmitterSetOverrideRayDirections(VAEmitter* emitter, VAVector* directions, int count);

/* Sets the gain formula used to compute audio gain from occlusion/permeation energy. Pass NULL to reset to the default formula.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if the formula produces NaN or Infinity during validation. */
EXPORT_API VAResult vaEmitterSetGainFormula(VAEmitter* emitter, VAEmitterGainFormula formula);
/* Sets the gain formula used to compute ambient audio gain. Pass NULL to reset to the default formula.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if the formula produces NaN or Infinity during validation. */
EXPORT_API VAResult vaEmitterSetAmbientGainFormula(VAEmitter* emitter, VAEmitterGainFormula formula);

/* True if reverb ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterReverbEnabled(const VAEmitter* emitter);
/* True if occlusion ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterOcclusionEnabled(const VAEmitter* emitter);
/* True if permeation ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterPermeationEnabled(const VAEmitter* emitter);
/* True if ambient occlusion ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterAmbientOcclusionEnabled(const VAEmitter* emitter);
/* True if ambient permeation ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterAmbientPermeationEnabled(const VAEmitter* emitter);
/* True if visualisation ray count and bounce count are both greater than zero. */
EXPORT_API bool vaEmitterVisualisationEnabled(const VAEmitter* emitter);
/* False if all ray counts and/or bounce counts are set to 0. */
EXPORT_API bool vaEmitterCastsRays(const VAEmitter* emitter);
/* Emitters outside the world bounds will not be raytraced; set clampPosition to true to keep them inside the world. */
EXPORT_API bool vaEmitterWithinWorldBounds(const VAEmitter* emitter);

/* Invoked after this emitter casts its rays for the first time. */
EXPORT_API void vaEmitterSetOnRaytracingCompleteCallback(VAEmitter* emitter, VAEmitterOnRaytracingComplete callback);
/* Invoked when another emitter raytraces this emitter for the first time. */
EXPORT_API void vaEmitterSetOnRaytracedByAnotherEmitterCallback(VAEmitter* emitter, VAEmitterOnRaytracedByAnotherEmitter callback);
/* Invoked with bounce data produced by visualisation rays; do not modify the array or access it outside the callback. */
EXPORT_API void vaEmitterSetVisualisationCallback(VAEmitter* emitter, VAEmitterVisualisationCallback callback);
/* Invoked for informational log messages from this emitter. */
EXPORT_API void vaEmitterSetLogCallback(VAEmitter* emitter, VALogCallback callback);
/* Invoked for error log messages from this emitter. */
EXPORT_API void vaEmitterSetLogErrorCallback(VAEmitter* emitter, VALogCallback callback);

/* World-space position of this emitter. */
EXPORT_API void vaEmitterSetPosition(VAEmitter* emitter, VAVector position);
/* World-space position of this emitter. */
EXPORT_API VAVector vaEmitterGetPosition(const VAEmitter* emitter);
/* When true, the emitter has not yet received its first raytracing result. */
EXPORT_API bool vaEmitterGetInitialising(const VAEmitter* emitter);
/* Whether this emitter's EAX is blended into grouped EAX. Set to false for listener emitters. */
EXPORT_API void vaEmitterSetAffectsGroupedEax(VAEmitter* emitter, bool value);
/* Whether this emitter's EAX is blended into grouped EAX. */
EXPORT_API bool vaEmitterGetAffectsGroupedEax(const VAEmitter* emitter);
/* Index into the world's grouped eax list (-1 = none). */
EXPORT_API int vaEmitterGetGroupedEaxIndex(const VAEmitter* emitter);
/* Read-only percentage of energy in ambient occlusion rays that reached the edge of the world (0–1). */
EXPORT_API float vaEmitterGetOutsidePercent(const VAEmitter* emitter);
/* Whether this emitter is used as a reference point for calculating relative reverb gain and direction. */
EXPORT_API void vaEmitterSetHasRelativeReverb(VAEmitter* emitter, bool value);
EXPORT_API bool vaEmitterGetHasRelativeReverb(const VAEmitter* emitter);
/* Lower bound of the relative reverb blend range. */
EXPORT_API void vaEmitterSetRelativeReverbInnerThreshold(VAEmitter* emitter, float value);
EXPORT_API float vaEmitterGetRelativeReverbInnerThreshold(const VAEmitter* emitter);
/* Upper bound of the relative reverb blend range. */
EXPORT_API void vaEmitterSetRelativeReverbOuterThreshold(VAEmitter* emitter, float value);
EXPORT_API float vaEmitterGetRelativeReverbOuterThreshold(const VAEmitter* emitter);
/* When true, position is clamped to world bounds to prevent going out of bounds. */
EXPORT_API void vaEmitterSetClampPosition(VAEmitter* emitter, bool value);
EXPORT_API bool vaEmitterGetClampPosition(const VAEmitter* emitter);
/* Seed used to randomise scattering vectors. */
EXPORT_API void vaEmitterSetScatteringSeed(VAEmitter* emitter, int value);
EXPORT_API int vaEmitterGetScatteringSeed(const VAEmitter* emitter);
/* Display name for debugging and logging. The emitter copies the string internally, so the caller does not need to keep the buffer alive after this call. */
EXPORT_API void vaEmitterSetName(VAEmitter* emitter, const char* name);
/* Returns the emitter's internal copy of the name. The pointer is valid until the next call to vaEmitterSetName or vaEmitterDestroy; do not free it. */
EXPORT_API const char* vaEmitterGetName(const VAEmitter* emitter);
/* User-defined integer tag for categorising emitters. */
EXPORT_API void vaEmitterSetType(VAEmitter* emitter, int value);
/* User-defined integer tag for categorising emitters. */
EXPORT_API int vaEmitterGetType(const VAEmitter* emitter);

/* Number of reverb rays cast per frame. */
EXPORT_API int vaEmitterGetReverbRayCount(const VAEmitter* emitter);
/* Maximum number of bounces per reverb ray. */
EXPORT_API int vaEmitterGetReverbBounceCount(const VAEmitter* emitter);
/* Number of occlusion rays cast per frame. */
EXPORT_API int vaEmitterGetOcclusionRayCount(const VAEmitter* emitter);
/* Maximum number of bounces per occlusion ray. */
EXPORT_API int vaEmitterGetOcclusionBounceCount(const VAEmitter* emitter);
/* Number of permeation rays cast per frame. */
EXPORT_API int vaEmitterGetPermeationRayCount(const VAEmitter* emitter);
/* Maximum number of bounces per permeation ray. */
EXPORT_API int vaEmitterGetPermeationBounceCount(const VAEmitter* emitter);
/* Number of ambient occlusion rays cast per frame. */
EXPORT_API int vaEmitterGetAmbientOcclusionRayCount(const VAEmitter* emitter);
/* Maximum number of bounces per ambient occlusion ray. */
EXPORT_API int vaEmitterGetAmbientOcclusionBounceCount(const VAEmitter* emitter);
/* Number of ambient permeation rays cast per frame. */
EXPORT_API int vaEmitterGetAmbientPermeationRayCount(const VAEmitter* emitter);
/* Maximum number of bounces per ambient permeation ray. */
EXPORT_API int vaEmitterGetAmbientPermeationBounceCount(const VAEmitter* emitter);
/* Number of visualisation rays cast per visualisation update. */
EXPORT_API int vaEmitterGetVisualisationRayCount(const VAEmitter* emitter);
/* Number of visualisation rays cast per visualisation update. */
EXPORT_API void vaEmitterSetVisualisationRayCount(VAEmitter* emitter, int value);
/* Maximum number of bounces per visualisation ray. */
EXPORT_API int vaEmitterGetVisualisationBounceCount(const VAEmitter* emitter);
/* Maximum number of bounces per visualisation ray. */
EXPORT_API void vaEmitterSetVisualisationBounceCount(VAEmitter* emitter, int value);
/* Interval in milliseconds between visualisation ray updates. */
EXPORT_API int vaEmitterGetVisualisationUpdateFrequency(const VAEmitter* emitter);
/* Interval in milliseconds between visualisation ray updates. */
EXPORT_API void vaEmitterSetVisualisationUpdateFrequency(VAEmitter* emitter, int value);
/* Total number of trails (reverb + occlusion + permeation) owned by this emitter. */
EXPORT_API int vaEmitterGetTrailCount(const VAEmitter* emitter);
/* Total bounce budget per trail across all ray types. */
EXPORT_API int vaEmitterGetTrailBounceCount(const VAEmitter* emitter);
/* How long (ms) the echogram records data. Returning reverb rays after this period are ignored. Defaults to 5000ms. */
EXPORT_API int vaEmitterGetMaxEchogramTime(const VAEmitter* emitter);
/* How long (ms) the echogram records data. Returning reverb rays after this period are ignored. Defaults to 5000ms. */
EXPORT_API void vaEmitterSetMaxEchogramTime(VAEmitter* emitter, int value);
/* Length (ms) of each entry in the echogram. Defaults to 50ms. */
EXPORT_API int vaEmitterGetEchogramGranularity(const VAEmitter* emitter);
/* Length (ms) of each entry in the echogram. Defaults to 50ms. */
EXPORT_API void vaEmitterSetEchogramGranularity(VAEmitter* emitter, int value);
/* Number of trails rebuilt from scratch each frame to prevent staleness when the emitter moves. Minimum 0. */
EXPORT_API int vaEmitterGetRefreshRayCount(const VAEmitter* emitter);
/* Number of trails rebuilt from scratch each frame to prevent staleness when the emitter moves. Minimum 0. */
EXPORT_API void vaEmitterSetRefreshRayCount(VAEmitter* emitter, int value);
/* A trail is re-created when an old bounce position is this far from the new position. Minimum 0. */
EXPORT_API float vaEmitterGetRefreshDistanceThreshold(const VAEmitter* emitter);
/* A trail is re-created when an old bounce position is this far from the new position. Minimum 0. */
EXPORT_API void vaEmitterSetRefreshDistanceThreshold(VAEmitter* emitter, float value);
/* Amount of returning energy required for reverb to be at maximum volume. Defaults to 100. */
EXPORT_API float vaEmitterGetReverbEnergyCap(const VAEmitter* emitter);

/* Percentage of occlusion energy required for the emitter to be at full volume. Defaults to 15% of occlusionRayCount. */
EXPORT_API float vaEmitterGetOcclusionEnergyCap(const VAEmitter* emitter);
/* Percentage of occlusion energy required for the emitter to be at full volume. Defaults to 15% of occlusionRayCount.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if value is NaN or Infinity, VA_OUT_OF_RANGE if value is less than 0. */
EXPORT_API VAResult vaEmitterSetOcclusionEnergyCap(VAEmitter* emitter, float value);

/* Percentage of permeation energy required for the emitter to be at full volume. Defaults to 15% of permeationRayCount * permeationBounceCount. */
EXPORT_API float vaEmitterGetPermeationEnergyCap(const VAEmitter* emitter);
/* Percentage of permeation energy required for the emitter to be at full volume. Defaults to 15% of permeationRayCount * permeationBounceCount.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if value is NaN or Infinity, VA_OUT_OF_RANGE if value is less than 0. */
EXPORT_API VAResult vaEmitterSetPermeationEnergyCap(VAEmitter* emitter, float value);

/* Percentage of ambient occlusion energy required for the emitter to be at full volume. */
EXPORT_API float vaEmitterGetAmbientOcclusionEnergyCap(const VAEmitter* emitter);
/* Percentage of ambient occlusion energy required for the emitter to be at full volume.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if value is NaN or Infinity, VA_OUT_OF_RANGE if value is less than 0. */
EXPORT_API VAResult vaEmitterSetAmbientOcclusionEnergyCap(VAEmitter* emitter, float value);

/* Percentage of ambient permeation energy required for the emitter to be at full volume. */
EXPORT_API float vaEmitterGetAmbientPermeationEnergyCap(const VAEmitter* emitter);
/* Percentage of ambient permeation energy required for the emitter to be at full volume.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if value is NaN or Infinity, VA_OUT_OF_RANGE if value is less than 0. */
EXPORT_API VAResult vaEmitterSetAmbientPermeationEnergyCap(VAEmitter* emitter, float value);
/* Number of target emitter slots pre-allocated. Minimum 0. */
EXPORT_API int vaEmitterGetReservedEmitterCount(const VAEmitter* emitter);
/* Number of target emitter slots pre-allocated. Minimum 0. */
EXPORT_API void vaEmitterSetReservedEmitterCount(VAEmitter* emitter, int value);
/* Energy threshold below which permeation rays are cancelled to prevent unnecessary traversal. */
EXPORT_API float vaEmitterGetMinimumPermeationEnergy(const VAEmitter* emitter);

/* Processed reverb results; only valid after raytracing has run at least once. */
EXPORT_API VAProcessedReverb* vaEmitterGetProcessedReverb(const VAEmitter* emitter);
/* EAX reverb results; only valid after raytracing has run at least once. */
EXPORT_API VAEAXReverb* vaEmitterGetEAX(const VAEmitter* emitter);
/* LF and HF gain for ambient sounds to apply to a low pass filter. */
EXPORT_API VALowPassFilter* vaEmitterGetAmbientFilter(const VAEmitter* emitter);

// Material type enum. First 23 values contain default materials. First 1000 values are reserved.
typedef enum VAMaterialType
{
    VAMaterialAir = 0,
    VAMaterialBrick,
    VAMaterialCloth,
    VAMaterialConcrete,
    VAMaterialConcretePolished,
    VAMaterialDirt,
    VAMaterialGlass,
    VAMaterialGrass,
    VAMaterialGravel,
    VAMaterialGyprock,
    VAMaterialIce,
    VAMaterialLeaf,
    VAMaterialMarble,
    VAMaterialMetal,
    VAMaterialMud,
    VAMaterialRock,
    VAMaterialSand,
    VAMaterialSnow,
    VAMaterialTile,
    VAMaterialTree,
    VAMaterialWater,
    VAMaterialWoodIndoor,
    VAMaterialWoodOutdoor,
    VAMaterialTypeCount
} VAMaterialType;

/* A capsule primitive for raytracing */
typedef struct VACapsulePrimitive VACapsulePrimitive;
/* Creates a new CapsulePrimitive with default values */
EXPORT_API VACapsulePrimitive* vaCapsulePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaCapsulePrimitiveDestroy(VACapsulePrimitive* capsule);

/* Radius of the capsule cylinder and end caps. */
EXPORT_API float vaCapsulePrimitiveGetRadius(const VACapsulePrimitive* capsule);
/* Length of the capsule cylinder, excluding end caps. */
EXPORT_API float vaCapsulePrimitiveGetLength(const VACapsulePrimitive* capsule);
/* World transform of the capsule. */
EXPORT_API const VAMatrix* vaCapsulePrimitiveGetTransform(const VACapsulePrimitive* capsule);

/* Radius of the capsule cylinder and end caps. */
EXPORT_API void vaCapsulePrimitiveSetRadius(VACapsulePrimitive* capsule, float radius);
/* Length of the capsule cylinder, excluding end caps. */
EXPORT_API void vaCapsulePrimitiveSetLength(VACapsulePrimitive* capsule, float length);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaCapsulePrimitiveSetTransform(VACapsulePrimitive* capsule, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaCapsulePrimitiveGetMaterial(const VACapsulePrimitive* capsule);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaCapsulePrimitiveSetMaterial(VACapsulePrimitive* capsule, VAMaterialType material);

/* A cone primitive for raytracing */
typedef struct VAConePrimitive VAConePrimitive;
/* Creates a new ConePrimitive with default values */
EXPORT_API VAConePrimitive* vaConePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaConePrimitiveDestroy(VAConePrimitive* cone);

/* Radius of the cone base. */
EXPORT_API float vaConePrimitiveGetRadius(const VAConePrimitive* cone);
/* Height of the cone. */
EXPORT_API float vaConePrimitiveGetHeight(const VAConePrimitive* cone);
/* World transform of the cone. */
EXPORT_API const VAMatrix* vaConePrimitiveGetTransform(const VAConePrimitive* cone);

/* Radius of the cone base. */
EXPORT_API void vaConePrimitiveSetRadius(VAConePrimitive* cone, float radius);
/* Height of the cone. */
EXPORT_API void vaConePrimitiveSetHeight(VAConePrimitive* cone, float height);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaConePrimitiveSetTransform(VAConePrimitive* cone, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaConePrimitiveGetMaterial(const VAConePrimitive* cone);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaConePrimitiveSetMaterial(VAConePrimitive* cone, VAMaterialType material);

/* A cylinder primitive for raytracing */
typedef struct VACylinderPrimitive VACylinderPrimitive;
/* Creates a new CylinderPrimitive with default values */
EXPORT_API VACylinderPrimitive* vaCylinderPrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaCylinderPrimitiveDestroy(VACylinderPrimitive* cylinder);

/* Radius of the cylinder. */
EXPORT_API float vaCylinderPrimitiveGetRadius(const VACylinderPrimitive* cylinder);
/* Length of the cylinder along its axis. */
EXPORT_API float vaCylinderPrimitiveGetLength(const VACylinderPrimitive* cylinder);
/* World transform of the cylinder. */
EXPORT_API const VAMatrix* vaCylinderPrimitiveGetTransform(const VACylinderPrimitive* cylinder);

/* Radius of the cylinder. */
EXPORT_API void vaCylinderPrimitiveSetRadius(VACylinderPrimitive* cylinder, float radius);
/* Length of the cylinder along its axis. */
EXPORT_API void vaCylinderPrimitiveSetLength(VACylinderPrimitive* cylinder, float length);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaCylinderPrimitiveSetTransform(VACylinderPrimitive* cylinder, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaCylinderPrimitiveGetMaterial(const VACylinderPrimitive* cylinder);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaCylinderPrimitiveSetMaterial(VACylinderPrimitive* cylinder, VAMaterialType material);

/* A disk primitive for raytracing */
typedef struct VADiskPrimitive VADiskPrimitive;
/* Creates a new DiskPrimitive with default values */
EXPORT_API VADiskPrimitive* vaDiskPrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaDiskPrimitiveDestroy(VADiskPrimitive* disk);

/* Radius of the disk. */
EXPORT_API float vaDiskPrimitiveGetRadius(const VADiskPrimitive* disk);
/* World transform of the disk. */
EXPORT_API const VAMatrix* vaDiskPrimitiveGetTransform(const VADiskPrimitive* disk);

/* Radius of the disk. */
EXPORT_API void vaDiskPrimitiveSetRadius(VADiskPrimitive* disk, float radius);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaDiskPrimitiveSetTransform(VADiskPrimitive* disk, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaDiskPrimitiveGetMaterial(const VADiskPrimitive* disk);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaDiskPrimitiveSetMaterial(VADiskPrimitive* disk, VAMaterialType material);

/* A half-sphere primitive for raytracing */
typedef struct VAHalfSpherePrimitive VAHalfSpherePrimitive;
/* Creates a new HalfSpherePrimitive with default values */
EXPORT_API VAHalfSpherePrimitive* vaHalfSpherePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaHalfSpherePrimitiveDestroy(VAHalfSpherePrimitive* halfSphere);

/* Radius of the half-sphere. */
EXPORT_API float vaHalfSpherePrimitiveGetRadius(VAHalfSpherePrimitive* halfSphere);
/* World transform of the half-sphere. */
EXPORT_API const VAMatrix* vaHalfSpherePrimitiveGetTransform(VAHalfSpherePrimitive* halfSphere);

/* Radius of the half-sphere. */
EXPORT_API void vaHalfSpherePrimitiveSetRadius(VAHalfSpherePrimitive* halfSphere, float radius);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaHalfSpherePrimitiveSetTransform(VAHalfSpherePrimitive* halfSphere, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaHalfSpherePrimitiveGetMaterial(const VAHalfSpherePrimitive* halfSphere);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaHalfSpherePrimitiveSetMaterial(VAHalfSpherePrimitive* halfSphere, VAMaterialType material);

/* A triangle mesh whose BVH is built once and can be shared across multiple VAMeshPrimitive instances */
typedef struct VAMesh VAMesh;

/* Creates a new Mesh from a vertex buffer. vertices is the vertex buffer (3 per triangle), vertex_count is the total vertex count,
   min_bounds/max_bounds are the pre-computed local-space AABB */
EXPORT_API VAMesh* vaMeshCreate(const VAVector* vertices, int vertexCount, VAVector minBounds, VAVector maxBounds);

/* Frees all resources owned by this mesh */
EXPORT_API VAResult vaMeshDestroy(VAMesh* mesh);

/* A triangle mesh primitive for raytracing */
typedef struct VAMeshPrimitive VAMeshPrimitive;
/* Creates a new MeshPrimitive. vertices is the vertex buffer (3 per triangle), vertex_count is total vertex count,
   min_bounds/max_bounds are the pre-computed AABB, transform is the initial world transform */
EXPORT_API VAMeshPrimitive* vaMeshPrimitiveCreate(VAMaterialType material, const VAVector* vertices, int vertexCount, VAVector minBounds, VAVector maxBounds, const VAMatrix* transform);

/* Creates a new MeshPrimitive that shares geometry from a VAMesh. The BVH is built once in the VAMesh and reused by every instance. */
EXPORT_API VAMeshPrimitive* vaMeshPrimitiveCreateFromMesh(VAMaterialType material, VAMesh* mesh, const VAMatrix* transform);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaMeshPrimitiveDestroy(VAMeshPrimitive* primitive);

/* World transform matrix for this mesh. */
EXPORT_API const VAMatrix* vaMeshPrimitiveGetTransform(const VAMeshPrimitive* primitive);
/* Sets the world transform matrix; updates all triangle bounds. */
EXPORT_API void vaMeshPrimitiveSetTransform(VAMeshPrimitive* primitive, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaMeshPrimitiveGetMaterial(const VAMeshPrimitive* cylinder);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaMeshPrimitiveSetMaterial(VAMeshPrimitive* cylinder, VAMaterialType material);

/* Whether sound can permeate through this mesh. Set to true for watertight meshes. */
EXPORT_API bool vaMeshPrimitiveGetSupports3DPermeation(const VAMeshPrimitive* primitive);
/* Whether sound can permeate through this mesh. Set to true for watertight meshes. */
EXPORT_API void vaMeshPrimitiveSetSupports3DPermeation(VAMeshPrimitive* primitive, bool supports3DPermeation);

/* An infinite plane primitive for raytracing */
typedef struct VAPlanePrimitive VAPlanePrimitive;
/* Creates a new PlanePrimitive with default values */
EXPORT_API VAPlanePrimitive* vaPlanePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaPlanePrimitiveDestroy(VAPlanePrimitive* plane);

/* Width of the plane. */
EXPORT_API float vaPlanePrimitiveGetWidth(VAPlanePrimitive* plane);
/* Height of the plane. */
EXPORT_API float vaPlanePrimitiveGetHeight(VAPlanePrimitive* plane);
/* World transform of the plane. */
EXPORT_API const VAMatrix* vaPlanePrimitiveGetTransform(VAPlanePrimitive* plane);

/* Width of the plane. */
EXPORT_API void vaPlanePrimitiveSetWidth(VAPlanePrimitive* plane, float width);
/* Height of the plane. */
EXPORT_API void vaPlanePrimitiveSetHeight(VAPlanePrimitive* plane, float height);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaPlanePrimitiveSetTransform(VAPlanePrimitive* plane, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaPlanePrimitiveGetMaterial(const VAPlanePrimitive* plane);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaPlanePrimitiveSetMaterial(VAPlanePrimitive* plane, VAMaterialType material);

/* A rectangular prism primitive for raytracing */
typedef struct VAPrismPrimitive VAPrismPrimitive;
/* Creates a new PrismPrimitive with default values */
EXPORT_API VAPrismPrimitive* vaPrismPrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaPrismPrimitiveDestroy(VAPrismPrimitive* prism);

/* Width, height, and depth of the prism. */
EXPORT_API VAVector vaPrismPrimitiveGetSize(const VAPrismPrimitive* prism);
/* World transform of the prism. */
EXPORT_API const VAMatrix* vaPrismPrimitiveGetTransform(const VAPrismPrimitive* prism);

/* Width, height, and depth of the prism. */
EXPORT_API void vaPrismPrimitiveSetSize(VAPrismPrimitive* prism, VAVector size);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaPrismPrimitiveSetTransform(VAPrismPrimitive* prism, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaPrismPrimitiveGetMaterial(const VAPrismPrimitive* prism);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaPrismPrimitiveSetMaterial(VAPrismPrimitive* prism, VAMaterialType material);

/* A rectangular cone primitive for raytracing */
typedef struct VARectangularConePrimitive VARectangularConePrimitive;
/* Creates a new RectangularConePrimitive with default values */
EXPORT_API VARectangularConePrimitive* vaRectangularConePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaRectangularConePrimitiveDestroy(VARectangularConePrimitive* rectCone);

/* Width of the rectangular cone base. */
EXPORT_API float vaRectangularConePrimitiveGetWidth(VARectangularConePrimitive* rectCone);
/* Length of the rectangular cone base. */
EXPORT_API float vaRectangularConePrimitiveGetLength(VARectangularConePrimitive* rectCone);
/* Height of the rectangular cone. */
EXPORT_API float vaRectangularConePrimitiveGetHeight(VARectangularConePrimitive* rectCone);
/* World transform of the rectangular cone. */
EXPORT_API const VAMatrix* vaRectangularConePrimitiveGetTransform(VARectangularConePrimitive* rectCone);

/* Width of the rectangular cone base. */
EXPORT_API void vaRectangularConePrimitiveSetWidth(VARectangularConePrimitive* rectCone, float width);
/* Length of the rectangular cone base. */
EXPORT_API void vaRectangularConePrimitiveSetLength(VARectangularConePrimitive* rectCone, float length);
/* Height of the rectangular cone. */
EXPORT_API void vaRectangularConePrimitiveSetHeight(VARectangularConePrimitive* rectCone, float height);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaRectangularConePrimitiveSetTransform(VARectangularConePrimitive* rectCone, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaRectangularConePrimitiveGetMaterial(const VARectangularConePrimitive* rectCone);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaRectangularConePrimitiveSetMaterial(VARectangularConePrimitive* rectCone, VAMaterialType material);

/* A sphere primitive for raytracing */
typedef struct VASpherePrimitive VASpherePrimitive;
/* Creates a new SpherePrimitive with default values */
EXPORT_API VASpherePrimitive* vaSpherePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaSpherePrimitiveDestroy(VASpherePrimitive* sphere);

/* Center of the sphere. */
EXPORT_API VAVector vaSpherePrimitiveGetCenter(VASpherePrimitive* sphere);
/* Radius of the sphere. */
EXPORT_API float vaSpherePrimitiveGetRadius(VASpherePrimitive* sphere);

/* Center of the sphere. */
EXPORT_API void vaSpherePrimitiveSetCenter(VASpherePrimitive* sphere, VAVector center);
/* Radius of the sphere. */
EXPORT_API void vaSpherePrimitiveSetRadius(VASpherePrimitive* sphere, float radius);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaSpherePrimitiveGetMaterial(const VASpherePrimitive* sphere);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaSpherePrimitiveSetMaterial(VASpherePrimitive* sphere, VAMaterialType material);

/* A single-triangle primitive for raytracing */
typedef struct VATrianglePrimitive VATrianglePrimitive;
/* Creates a new TrianglePrimitive with default values */
EXPORT_API VATrianglePrimitive* vaTrianglePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaTrianglePrimitiveDestroy(VATrianglePrimitive* triangle);

/* First vertex position. */
EXPORT_API VAVector vaTrianglePrimitiveGetPosition0(VATrianglePrimitive* triangle);
/* Second vertex position. */
EXPORT_API VAVector vaTrianglePrimitiveGetPosition1(VATrianglePrimitive* triangle);
/* Third vertex position. */
EXPORT_API VAVector vaTrianglePrimitiveGetPosition2(VATrianglePrimitive* triangle);

/* First vertex position. */
EXPORT_API void vaTrianglePrimitiveSetPosition0(VATrianglePrimitive* triangle, VAVector position);
/* Second vertex position. */
EXPORT_API void vaTrianglePrimitiveSetPosition1(VATrianglePrimitive* triangle, VAVector position);
/* Third vertex position. */
EXPORT_API void vaTrianglePrimitiveSetPosition2(VATrianglePrimitive* triangle, VAVector position);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaTrianglePrimitiveGetMaterial(const VATrianglePrimitive* triangle);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaTrianglePrimitiveSetMaterial(VATrianglePrimitive* triangle, VAMaterialType material);

/* A triangular cone primitive for raytracing */
typedef struct VATriangularConePrimitive VATriangularConePrimitive;
/* Creates a new TriangularConePrimitive with default values */
EXPORT_API VATriangularConePrimitive* vaTriangularConePrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaTriangularConePrimitiveDestroy(VATriangularConePrimitive* cone);

/* Radius of the triangular cone base. */
EXPORT_API float vaTriangularConePrimitiveGetRadius(VATriangularConePrimitive* cone);
/* Height of the triangular cone. */
EXPORT_API float vaTriangularConePrimitiveGetHeight(VATriangularConePrimitive* cone);
/* World transform of the triangular cone. */
EXPORT_API const VAMatrix* vaTriangularConePrimitiveGetTransform(VATriangularConePrimitive* cone);

/* Radius of the triangular cone base. */
EXPORT_API void vaTriangularConePrimitiveSetRadius(VATriangularConePrimitive* cone, float radius);
/* Height of the triangular cone. */
EXPORT_API void vaTriangularConePrimitiveSetHeight(VATriangularConePrimitive* cone, float height);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaTriangularConePrimitiveSetTransform(VATriangularConePrimitive* cone, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaTriangularConePrimitiveGetMaterial(const VATriangularConePrimitive* cone);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaTriangularConePrimitiveSetMaterial(VATriangularConePrimitive* cone, VAMaterialType material);

/* A triangular prism primitive for raytracing */
typedef struct VATriangularPrismPrimitive VATriangularPrismPrimitive;
/* Creates a new TriangularPrismPrimitive with default values */
EXPORT_API VATriangularPrismPrimitive* vaTriangularPrismPrimitiveCreate(void);
/* Frees all resources owned by this primitive */
EXPORT_API VAResult vaTriangularPrismPrimitiveDestroy(VATriangularPrismPrimitive* prism);

/* Radius of the triangular prism cross-section. */
EXPORT_API float vaTriangularPrismPrimitiveGetRadius(VATriangularPrismPrimitive* prism);
/* Length of the triangular prism along its axis. */
EXPORT_API float vaTriangularPrismPrimitiveGetLength(VATriangularPrismPrimitive* prism);
/* World transform of the triangular prism. */
EXPORT_API const VAMatrix* vaTriangularPrismPrimitiveGetTransform(VATriangularPrismPrimitive* prism);

/* Radius of the triangular prism cross-section. */
EXPORT_API void vaTriangularPrismPrimitiveSetRadius(VATriangularPrismPrimitive* prism, float radius);
/* Length of the triangular prism along its axis. */
EXPORT_API void vaTriangularPrismPrimitiveSetLength(VATriangularPrismPrimitive* prism, float length);

/* Sets the world transform. Must not contain scale components. */
EXPORT_API void vaTriangularPrismPrimitiveSetTransform(VATriangularPrismPrimitive* prism, const VAMatrix* transform);

/* Material assigned to this primitive. */
EXPORT_API VAMaterialType vaTriangularPrismPrimitiveGetMaterial(const VATriangularPrismPrimitive* prism);
/* Material assigned to this primitive. Must not be VAMaterialAir. */
EXPORT_API void vaTriangularPrismPrimitiveSetMaterial(VATriangularPrismPrimitive* prism, VAMaterialType material);

typedef struct VAWorld VAWorld;

/* Virtual function table for overriding EAX parameter calculations */
typedef struct VACustomEAXFormulas VACustomEAXFormulas;

// Function pointer types for virtual methods

/* Called once with echogram data before other calculations. Use to pre-compute shared state */
typedef void (*CustomEAXFormulas_InitialiseFunc)(VACustomEAXFormulas* formulas, VAProcessedReverb* processed, float totalReturnedEnergyLF, float totalReturnedEnergyHF, float echogramGranularity, float* echogramLF, float* echogramHF, float* echogramAverage, int echogramLength);

/* Returns the computed diffusion value (0–1) */
typedef float (*CustomEAXFormulas_CalculateDiffusionFunc)(VACustomEAXFormulas* formulas);

/* Returns the computed density value (0–1) */
typedef float (*CustomEAXFormulas_CalculateDensityFunc)(VACustomEAXFormulas* formulas);

/* Returns the reflections delay in seconds. energy_threshold is the peak energy level */
typedef float (*CustomEAXFormulas_CalculateReflectionsDelayFunc)(VACustomEAXFormulas* formulas, float energyThreshold);

/* Returns the late reverb delay in seconds */
typedef float (*CustomEAXFormulas_CalculateLateReverbDelayFunc)(VACustomEAXFormulas* formulas);

/* Writes LF and HF gains to out_lf_gain and out_hf_gain. reference_energy is the peak energy */
typedef void (*CustomEAXFormulas_CalculateFrequencyGainsFunc)(VACustomEAXFormulas* formulas, float referenceEnergy, float* outLFGain, float* outHFGain);

/* Writes early and late gains to out_reflections_gain and out_late_reverb_gain */
typedef void (*CustomEAXFormulas_CalculateReflectionsAndLateReverbGainFunc)(VACustomEAXFormulas* formulas, float earlyLateTransitionMs, float referenceEnergy, float* outReflectionsGain, float* outLateReverbGain);

/* Returns the RT60 decay time in seconds from the given echogram */
typedef float (*CustomEAXFormulas_CalculateRT60Func)(VACustomEAXFormulas* formulas, float* echogram, int echogramLength);

/* Attempts to calculate decay time; returns -1 if insufficient data */
typedef float (*CustomEAXFormulas_TryCalculateDecayTimeFunc)(VACustomEAXFormulas* formulas, float* echogramDb, int echogramDbLength, int peakIndex, float peakDb, float binDurationMs, float targetDecayDb, float scaleFactor);

struct VACustomEAXFormulas
{
    // Virtual function pointers
    CustomEAXFormulas_InitialiseFunc initialise;
    CustomEAXFormulas_CalculateDiffusionFunc calculateDiffusion;
    CustomEAXFormulas_CalculateDensityFunc calculateDensity;
    CustomEAXFormulas_CalculateReflectionsDelayFunc calculateReflectionsDelay;
    CustomEAXFormulas_CalculateLateReverbDelayFunc calculateLateReverbDelay;
    CustomEAXFormulas_CalculateFrequencyGainsFunc calculateFrequencyGains;
    CustomEAXFormulas_CalculateReflectionsAndLateReverbGainFunc calculateReflectionsAndLateReverbGain;
    CustomEAXFormulas_CalculateRT60Func calculateRt60;
    CustomEAXFormulas_TryCalculateDecayTimeFunc tryCalculateDecayTime;

    // Data members
    float* tempDiffusionCumulative;
    int tempDiffusionCumulativeLength;

    float* echogramDb;
    int echogramDbLength;

    float binDurationMs;

    VAProcessedReverb* processed;
    float totalReturnedEnergyLF;
    float totalReturnedEnergyHF;
    float totalReturnedEnergyAverage;
    float totalReturnedEnergy;
    float* echogramLF;
    float* echogramHF;
    float* echogramAverage;
    int echogramLength;

    float maxEnergy;

    VAWorld* world;
};

EXPORT_API VACustomEAXFormulas* vaCustomEaxFormulasCreate(void);
/* Frees all resources owned by these formulas. Returns VA_ERROR_IN_USE if the formulas are still assigned to a world. */
EXPORT_API VAResult vaCustomEaxFormulasDestroy(VACustomEAXFormulas* formulas);

/* Useful properties produced by reverb rays. */
typedef struct VAProcessedReverb
{
    // percentages
    float returnedPercent;        /* Percentage of energy that returned to the emitter. Calculated as raw.returned_total / (reverb_ray_count * reverb_bounce_count) */
    float outsidePercent;          /* Percentage of energy that escaped outside. Calculated as raw.outside_total / (reverb_ray_count * reverb_bounce_count) */

    // decay times
    float measuredDecayTimeLF;   /* Low-frequency reverberation decay time from the echogram, in seconds. Calculated using RT20/RT30/RT60 method */
    float measuredDecayTimeHF;   /* High-frequency reverberation decay time from the echogram, in seconds. Calculated using RT20/RT30/RT60 method */

    // scattering
    float materialRoughness;       /* Average roughness/scattering of all surfaces hit by rays */

    // absorption
    float materialAbsorptionLF;   /* Average low-frequency absorption of all surfaces hit by rays */
    float materialAbsorptionHF;   /* Average high-frequency absorption of all surfaces hit by rays */
} VAProcessedReverb;

/* Returns the average of material_absorption_lf and material_absorption_hf */
EXPORT_API float vaProcessedReverbGetMaterialAbsorption(const VAProcessedReverb* p);

// Custom air absorption formula. Takes distance in meters, returns energy loss as a percentage
typedef float (*VAAirAbsorptionCustomFormula)(float distanceMeters);

// Settings that control how sound energy is absorbed by the air over distance
typedef struct VAAirAbsorptionSettings VAAirAbsorptionSettings;

// Creates a new AirAbsorptionSettings with default values
EXPORT_API VAAirAbsorptionSettings* vaAirAbsorptionCreate();

// Returns true if all settings values are within valid ranges
EXPORT_API bool vaAirAbsorptionValidate(const VAAirAbsorptionSettings* settings);

// Setters — return VA_SUCCESS, VA_INVALID_VALUE if NaN/Infinity, or VA_OUT_OF_RANGE if out of range
// Relative humidity as a percentage (0–1)
EXPORT_API VAResult vaAirAbsorptionSetHumidity(VAAirAbsorptionSettings* settings, float value);
// Air temperature in degrees Celsius (must be > -273.15)
EXPORT_API VAResult vaAirAbsorptionSetTemperature(VAAirAbsorptionSettings* settings, float value);
// Atmospheric pressure in Pascals (must be > 0)
EXPORT_API VAResult vaAirAbsorptionSetPressure(VAAirAbsorptionSettings* settings, float value);
// Custom formula for low frequency sound. NULL to use default
EXPORT_API void vaAirAbsorptionSetCustomFormulaLF(VAAirAbsorptionSettings* settings, VAAirAbsorptionCustomFormula value);
// Custom formula for high frequency sound. NULL to use default
EXPORT_API void vaAirAbsorptionSetCustomFormulaHF(VAAirAbsorptionSettings* settings, VAAirAbsorptionCustomFormula value);

// Getters
// Relative humidity as a percentage (0–1)
EXPORT_API float vaAirAbsorptionGetHumidity(const VAAirAbsorptionSettings* settings);
// Air temperature in degrees Celsius
EXPORT_API float vaAirAbsorptionGetTemperature(const VAAirAbsorptionSettings* settings);
// Atmospheric pressure in Pascals
EXPORT_API float vaAirAbsorptionGetPressure(const VAAirAbsorptionSettings* settings);
// Custom LF formula. NULL means default is used
EXPORT_API VAAirAbsorptionCustomFormula vaAirAbsorptionGetCustomFormulaLF(const VAAirAbsorptionSettings* settings);
// Custom HF formula. NULL means default is used
EXPORT_API VAAirAbsorptionCustomFormula vaAirAbsorptionGetCustomFormulaHF(const VAAirAbsorptionSettings* settings);

// Frees all resources owned by this settings object
EXPORT_API void vaAirAbsorptionDestroy(VAAirAbsorptionSettings* settings);

typedef struct VAPrimitive VAPrimitive;
/* A standalone world with its own primitives, emitters, materials and settings */
/* Creates a new world */
EXPORT_API VAWorld* vaWorldCreate();
/* Waits for background thread to complete, then frees all resources. After calling this method, this world cannot be reused. */
EXPORT_API void vaWorldDestroy(VAWorld* world);

/* Updates the raytracing simulation. Call this method regularly to process raytracing results and submit new work.
   Does nothing if background raytracing threads are still running. When threads are idle, it:
   - Handles the last raytracing results, updating reverb objects and invoking OnRaytracedByAnotherEmitter callbacks
   - Applies new settings and resizes memory buffers if needed
   - Processes new, modified, and removed primitives
   - Starts raytracing again on background threads
   Must be called from the main thread. Calling this more frequently is safe and can reduce latency for emitter updates. */
EXPORT_API void vaWorldUpdate(VAWorld* world);
/* Blocks the calling thread until all background raytracing threads complete, then handles the results (updates reverb objects and invokes OnRaytracingComplete and OnRaytracedByAnotherEmitter callbacks for each emitter). */
EXPORT_API void vaWorldWait(VAWorld* world);

/* When set to true, vaWorldUpdate will stop submitting work to background threads.
   When vaWorldGetThreadsRunning() becomes false, it is safe to call vaWorldDestroy(). */
EXPORT_API void vaWorldSetPendingShutdown(VAWorld* world);
/* True while background raytracing threads are still running.
   After setting pending shutdown, continue calling vaWorldUpdate until this is false, then call vaWorldDestroy(). */
EXPORT_API bool vaWorldGetThreadsRunning(VAWorld* world);

/* Number of rays cast this frame. */
EXPORT_API int vaWorldGetRaysCastThisFrame(const VAWorld* world);

/* List of grouped EAX reverb properties for all emitters. Contains parameters compatible with EAX reverb effects. */
EXPORT_API const VAEAXReverb** vaWorldGetGroupedEAX(const VAWorld* world);

/* Current number of grouped EAX effects (may be less than the maximum). */
EXPORT_API int vaWorldGetGroupedEAXCount(const VAWorld* world);

// Primitive management — use the macros below, not these directly
EXPORT_API void vaWorldAddPrimitive_(VAWorld* world, void* primitive);
EXPORT_API void vaWorldRemovePrimitive_(VAWorld* world, void* primitive);

#define vaWorldAddPrimitive(world, p) _Generic((p), \
    VASpherePrimitive*:          vaWorldAddPrimitive_((world), (void*)(p)), \
    VACapsulePrimitive*:         vaWorldAddPrimitive_((world), (void*)(p)), \
    VAConePrimitive*:            vaWorldAddPrimitive_((world), (void*)(p)), \
    VACylinderPrimitive*:        vaWorldAddPrimitive_((world), (void*)(p)), \
    VADiskPrimitive*:            vaWorldAddPrimitive_((world), (void*)(p)), \
    VAHalfSpherePrimitive*:      vaWorldAddPrimitive_((world), (void*)(p)), \
    VAPlanePrimitive*:           vaWorldAddPrimitive_((world), (void*)(p)), \
    VAPrismPrimitive*:           vaWorldAddPrimitive_((world), (void*)(p)), \
    VARectangularConePrimitive*: vaWorldAddPrimitive_((world), (void*)(p)), \
    VATrianglePrimitive*:        vaWorldAddPrimitive_((world), (void*)(p)), \
    VATriangularConePrimitive*:  vaWorldAddPrimitive_((world), (void*)(p)), \
    VATriangularPrismPrimitive*: vaWorldAddPrimitive_((world), (void*)(p)), \
    VAMeshPrimitive*:            vaWorldAddPrimitive_((world), (void*)(p))  \
)
#define vaWorldRemovePrimitive(world, p) _Generic((p), \
    VASpherePrimitive*:          vaWorldRemovePrimitive_((world), (void*)(p)), \
    VACapsulePrimitive*:         vaWorldRemovePrimitive_((world), (void*)(p)), \
    VAConePrimitive*:            vaWorldRemovePrimitive_((world), (void*)(p)), \
    VACylinderPrimitive*:        vaWorldRemovePrimitive_((world), (void*)(p)), \
    VADiskPrimitive*:            vaWorldRemovePrimitive_((world), (void*)(p)), \
    VAHalfSpherePrimitive*:      vaWorldRemovePrimitive_((world), (void*)(p)), \
    VAPlanePrimitive*:           vaWorldRemovePrimitive_((world), (void*)(p)), \
    VAPrismPrimitive*:           vaWorldRemovePrimitive_((world), (void*)(p)), \
    VARectangularConePrimitive*: vaWorldRemovePrimitive_((world), (void*)(p)), \
    VATrianglePrimitive*:        vaWorldRemovePrimitive_((world), (void*)(p)), \
    VATriangularConePrimitive*:  vaWorldRemovePrimitive_((world), (void*)(p)), \
    VATriangularPrismPrimitive*: vaWorldRemovePrimitive_((world), (void*)(p)), \
    VAMeshPrimitive*:            vaWorldRemovePrimitive_((world), (void*)(p))  \
)

/* The minimum bounds of the world. Emitters outside the world will not be raytraced, and primitives fully outside these bounds will not affect raytracing. */
EXPORT_API VAVector vaWorldGetPosition(const VAWorld* world);

/* Set the world position. Returns VA_SUCCESS, or VA_INVALID_VALUE if the new value is NaN or Infinity. */
EXPORT_API VAResult vaWorldSetPosition(VAWorld* world, VAVector position);

/* The size of the world. Emitters outside the world will not be raytraced, and primitives fully outside these bounds will be ignored. */
EXPORT_API VAVector vaWorldGetSize(const VAWorld* world);

/* Set the world size. Returns VA_SUCCESS, or VA_INVALID_VALUE if the new value is NaN, Infinity or less than or equal to (0, 0, 0). */
EXPORT_API VAResult vaWorldSetSize(VAWorld* world, VAVector size);

/* The maximum bounds of the world (Position + Size). */
EXPORT_API VAVector vaWorldGetMaxBounds(const VAWorld* world);

/* Set the max bounds of the world. Returns VA_SUCCESS, or VA_INVALID_VALUE if the new value is NaN, Infinity or less than or equal to (0, 0, 0). */
EXPORT_API VAResult vaWorldSetMaxBounds(VAWorld* world, VAVector maxBounds);

/* Number of emitters in this world */
EXPORT_API int vaWorldGetEmitterCount(const VAWorld* world);
/* Total number of rays that could potentially be cast each frame across all emitters. */
EXPORT_API int vaWorldGetTotalPossibleRayCount(const VAWorld* world);
/* Percentage of rays reused from previous frames rather than cast this frame. Higher values indicate better performance. */
EXPORT_API float vaWorldGetRayCachePercent(const VAWorld* world);

/* The maximum number of grouped EAX reverb properties created for all emitters. Higher values increase accuracy but are more expensive to run. Must be >= 2. Returns VA_SUCCESS, or VA_OUT_OF_RANGE if value is less than 2. */
EXPORT_API VAResult vaWorldSetMaximumGroupedEAXCount(VAWorld* world, int value);
/* The number of work items to split trails across for load balancing. A higher value helps evenly distribute work across all threads. Must be >= 1. Returns VA_SUCCESS, or VA_OUT_OF_RANGE if value is less than 1. */
EXPORT_API VAResult vaWorldSetWorkItemCount(VAWorld* world, int value);
/* The maximum amount of threads that can run in parallel for this world. Must be >= 1. Returns VA_SUCCESS, or VA_OUT_OF_RANGE if value is less than 1. */
EXPORT_API VAResult vaWorldSetMaximumConcurrencyLevel(VAWorld* world, int value);

/* Gets meters per world unit. Affects air absorption and reverb calculation. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if <= 0. */
EXPORT_API VAResult vaWorldSetMetersPerUnit(VAWorld* world, float value);
/* Inverse speed of sound in seconds per meter. Defaults to 1.0f / 343.0f. Affects reverb calculation. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if <= 0. */
EXPORT_API VAResult vaWorldSetInverseSpeedOfSound(VAWorld* world, float value);
/* Low-frequency reference (Hz) for air absorption, reverb, and material scattering. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if <= 0. */
EXPORT_API VAResult vaWorldSetReferenceFrequencyLF(VAWorld* world, float value);
/* High-frequency reference (Hz) for air absorption, reverb, and material scattering. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if <= 0. */
EXPORT_API VAResult vaWorldSetReferenceFrequencyHF(VAWorld* world, float value);
/* Air absorption settings */
EXPORT_API void* vaWorldGetAirAbsorption(VAWorld* world);
/* Air absorption settings. Pass NULL to disable air absorption. Returns VA_SUCCESS, or VA_INVALID_VALUE if validation fails. */
EXPORT_API VAResult vaWorldSetAirAbsorption(VAWorld* world, VAAirAbsorptionSettings* value);

/* Relative humidity as a percentage (0–1). Returns VA_SUCCESS, VA_INVALID_VALUE if NaN/Infinity, or VA_OUT_OF_RANGE if out of range. */
EXPORT_API VAResult vaWorldSetAirAbsorptionHumidity(VAWorld* world, float value);
/* Air temperature in degrees Celsius (must be > -273.15). Returns VA_SUCCESS, VA_INVALID_VALUE if NaN/Infinity, or VA_OUT_OF_RANGE if out of range. */
EXPORT_API VAResult vaWorldSetAirAbsorptionTemperature(VAWorld* world, float value);
/* Atmospheric pressure in Pascals (must be > 0). Returns VA_SUCCESS, VA_INVALID_VALUE if NaN/Infinity, or VA_OUT_OF_RANGE if out of range. */
EXPORT_API VAResult vaWorldSetAirAbsorptionPressure(VAWorld* world, float value);
/* Custom LF air absorption formula. Takes distance in meters, returns energy loss as a percentage. NULL to use default. */
EXPORT_API void vaWorldSetAirAbsorptionCustomFormulaLF(VAWorld* world, VAAirAbsorptionCustomFormula value);
/* Custom HF air absorption formula. Takes distance in meters, returns energy loss as a percentage. NULL to use default. */
EXPORT_API void vaWorldSetAirAbsorptionCustomFormulaHF(VAWorld* world, VAAirAbsorptionCustomFormula value);

/* Custom formulas for calculating EAX properties (diffusion, density, etc). Pass NULL to use default formulas. */
EXPORT_API void vaWorldSetCustomEAXFormulas(VAWorld* world, VACustomEAXFormulas* formulas);
/* Whether emitters outside the world have 0 occlusion/permeation energy (true) or maximum energy (false). */
EXPORT_API void vaWorldSetEmittersOutsideTheWorldAreMuffled(VAWorld* world, bool value);
/* Whether the entire world is considered indoors or outdoors. When false, reverb rays stop checking for line-of-sight after hitting the world edge. Defaults to false. */
EXPORT_API bool vaWorldGetWorldIsIndoors(const VAWorld* world);
/* Whether the entire world is considered indoors or outdoors. When false, reverb rays stop checking for line-of-sight after hitting the world edge. Defaults to false. */
EXPORT_API void vaWorldSetWorldIsIndoors(VAWorld* world, bool value);


// TODO - move these above to be in pairs with the setters

/* The maximum number of grouped EAX reverb properties created for all emitters. */
EXPORT_API int vaWorldGetMaximumGroupedEAXCount(const VAWorld* world);
/* The number of work items to split trails across for load balancing. */
EXPORT_API int vaWorldGetWorkItemCount(const VAWorld* world);
/* The maximum amount of threads that can run in parallel for this world. */
EXPORT_API int vaWorldGetMaximumConcurrencyLevel(const VAWorld* world);
/* Whether the system is running in single-threaded mode. */
EXPORT_API bool vaWorldGetSingleThreaded(const VAWorld* world);
/* Whether the system is running in single-threaded mode. */
EXPORT_API void vaWorldSetSingleThreaded(VAWorld* world, bool value);
/* Gets meters per world unit. Affects air absorption and reverb calculation. */
EXPORT_API float vaWorldGetMetersPerUnit(const VAWorld* world);
/* Inverse speed of sound in seconds per meter. Defaults to 1.0f / 343.0f. Affects reverb calculation. */
EXPORT_API float vaWorldGetInverseSpeedOfSound(const VAWorld* world);
/* Low-frequency reference (Hz) for air absorption, reverb, and material scattering. */
EXPORT_API float vaWorldGetReferenceFrequencyLF(const VAWorld* world);
/* High-frequency reference (Hz) for air absorption, reverb, and material scattering. */
EXPORT_API float vaWorldGetReferenceFrequencyHF(const VAWorld* world);
/* Whether emitters outside the world have 0 occlusion/permeation energy (true) or maximum energy (false). */
EXPORT_API bool vaWorldGetEmittersOutsideTheWorldAreMuffled(const VAWorld* world);

// TODO
EXPORT_API float vaWorldGetEpsilon(const VAWorld* world);
// TODO
EXPORT_API void vaWorldSetEpsilon(VAWorld* world, float value);

// Material functions — materialId is a MaterialType or a custom integer ID registered via vaWorldCreateMaterial
/* Add a new material to this world. */
EXPORT_API void vaWorldCreateMaterial(VAWorld* world, int materialId);

/* Gets the percentage of low-frequency energy that is lost when a ray bounces off this material, in the range 0.0 to 1.0 */
EXPORT_API float vaWorldGetMaterialAbsorptionLF(const VAWorld* world, int materialId);

/* Gets the percentage of high-frequency energy that is lost when a ray bounces off this material, in the range 0.0 to 1.0 */
EXPORT_API float vaWorldGetMaterialAbsorptionHF(const VAWorld* world, int materialId);

/* Gets the scattering strength when a ray bounces off this material, in the range 0.0 to 1.0 (0.0 = no scattering, 1.0 skews the ray reflection direction by up to 90 degrees) */
EXPORT_API float vaWorldGetMaterialScattering(const VAWorld* world, int materialId);

/* Gets how much low-frequency energy is lost (in dB/meter) when a permeation ray passes through this primitive, in the range 0.0 or greater */
EXPORT_API float vaWorldGetMaterialTransmissionLF(const VAWorld* world, int materialId);

/* Gets how much high-frequency energy is lost (in dB/meter) when a permeation ray passes through this primitive, in the range 0.0 or greater */
EXPORT_API float vaWorldGetMaterialTransmissionHF(const VAWorld* world, int materialId);

/* Gets the percentage of low-frequency energy that is lost when a permeation ray passes through a flat VAPlanePrimitive, VADiskPrimitive, VATrianglePrimitive, or non-watertight VAMeshPrimitive, in the range 0.0 to 1.0 */
EXPORT_API float vaWorldGetMaterialPlaneTransmissionLF(const VAWorld* world, int materialId);

/* Gets the percentage of high-frequency energy that is lost when a permeation ray passes through a flat VAPlanePrimitive, VADiskPrimitive, VATrianglePrimitive, or non-watertight VAMeshPrimitive, in the range 0.0 to 1.0 */
EXPORT_API float vaWorldGetMaterialPlaneTransmissionHF(const VAWorld* world, int materialId);

/* Updates the percentage of low-frequency energy that is lost when a ray bounces off this material. Must be in the range 0.0 to 1.0. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0 or > 1.0. */
EXPORT_API VAResult vaWorldSetMaterialAbsorptionLF(VAWorld* world, int materialId, float value);

/* Updates the percentage of high-frequency energy that is lost when a ray bounces off this material. Must be in the range 0.0 to 1.0. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0 or > 1.0. */
EXPORT_API VAResult vaWorldSetMaterialAbsorptionHF(VAWorld* world, int materialId, float value);

/* Updates the scattering strength (0.0 = no scattering, 1.0 skews the ray reflection direction by up to 90 degrees). Must be in the range 0.0 to 1.0. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0 or > 1.0. */
EXPORT_API VAResult vaWorldSetMaterialScattering(VAWorld* world, int materialId, float value);

/* Updates how much low-frequency energy is lost (in dB/meter) when a permeation ray passes through this primitive. Must be 0.0 or greater. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0. */
EXPORT_API VAResult vaWorldSetMaterialTransmissionLF(VAWorld* world, int materialId, float value);

/* Updates how much high-frequency energy is lost (in dB/meter) when a permeation ray passes through this primitive. Must be 0.0 or greater. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0. */
EXPORT_API VAResult vaWorldSetMaterialTransmissionHF(VAWorld* world, int materialId, float value);

/* Updates the percentage of low-frequency energy that is lost when a permeation ray passes through a flat VAPlanePrimitive, VADiskPrimitive, VATrianglePrimitive, or non-watertight VAMeshPrimitive. Must be in the range 0.0 to 1.0. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0. */
EXPORT_API VAResult vaWorldSetMaterialPlaneTransmissionLF(VAWorld* world, int materialId, float value);

/* Updates the percentage of high-frequency energy that is lost when a permeation ray passes through a flat VAPlanePrimitive, VADiskPrimitive, VATrianglePrimitive, or non-watertight VAMeshPrimitive. Must be in the range 0.0 to 1.0. Returns VA_SUCCESS, VA_INVALID_VALUE if NaN or Infinity, or VA_OUT_OF_RANGE if < 0.0. */
EXPORT_API VAResult vaWorldSetMaterialPlaneTransmissionHF(VAWorld* world, int materialId, float value);

/* Average time (ms) spent by vaWorldUpdate() on the main thread */
EXPORT_API double vaWorldGetMainThreadTime(VAWorld* world);
/* Average time (ms) spent in background raytracing threads */
EXPORT_API double vaWorldGetRaytracingTime(VAWorld* world);
/* Average time (ms) spent in the preparation thread */
EXPORT_API double vaWorldGetPreparationTime(VAWorld* world);
/* Average time (ms) spent in the analysis thread */
EXPORT_API double vaWorldGetAnalysisTime(VAWorld* world);

/* Add an emitter to the world. Thread-safe.
   Returns VA_SUCCESS, VA_INVALID_VALUE if already added to this world, or VA_OUT_OF_RANGE if already added to another world. */
EXPORT_API VAResult vaWorldAddEmitter(VAWorld* world, VAEmitter* emitter);
/* Remove an emitter from the world. Thread-safe. This emitter's OnRaytracingComplete callback will not be invoked. */
EXPORT_API void vaWorldRemoveEmitter(VAWorld* world, VAEmitter* emitter);

/* Returns true when raytracing has run at least once, and it is safe to access reverb objects. */
EXPORT_API bool vaWorldGetReverbCalculated(const VAWorld* world);

/* This callback is invoked right after the background threads finish running. At this point, the results of the background threads have not been processed yet (EAX is not updated yet, low pass filters not updated, ambience not updated, etc). After this, onReverbUpdated will be invoked next. */
EXPORT_API void vaWorldSetOnRaytracingCompletedCallback(VAWorld* world, void (*callback)(void));

/* This callback is invoked after EAX reverb results are updated. This gives you a chance to update your EAX effects, so they can be applied to an emitter in it's onRaytracingComplete callback. After this, each emitter's callback are invoked, and then onRaytracingResultsHandled will be invoked next.*/
EXPORT_API void vaWorldSetOnReverbUpdatedCallback(VAWorld* world, void (*callback)(void));

/* This callback is invoked after all raytracing results (EAX, filters, etc) have been processed. After this, new emitter/primitive settings will be applied, and the background threads will start again. */
EXPORT_API void vaWorldSetOnRaytracingResultsHandledCallback(VAWorld* world, void (*callback)(void));

/* A custom logging callback. */
EXPORT_API void vaWorldSetLogCallback(VAWorld* world, VALogCallback callback);
/* Whether to log memory allocation warnings. */
EXPORT_API void vaWorldSetLogMemoryAllocationWarnings(VAWorld* world, bool value);

/* Converts a world-space vector to a listener-relative pan direction given the listener's pitch and yaw angles. */
EXPORT_API VAVector vaWorldCalculateListenerRelativePan(VAVector worldVector, float listenerPitch, float listenerYaw);

/* Exports all world settings, materials, primitives, and emitters to a binary file.
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if the file could not be opened or written. */
EXPORT_API VAResult vaWorldExport(VAWorld* world, const char* fileName);

/* Imports world settings, materials, primitives, and emitters from a binary file produced by vaWorldExport().
   Existing primitives and emitters are not removed before importing.
   outEmitters: set to a malloc'd array of pointers to each added emitter — caller must free (may be NULL).
   outEmitterCount: number of emitters added (may be NULL).
   Returns VA_SUCCESS on success, VA_INVALID_VALUE if the file could not be opened, the magic is wrong, or the version is unsupported. */
EXPORT_API VAResult vaWorldImport(VAWorld* world, const char* fileName, VAEmitter*** outEmitters, int* outEmitterCount);

#ifdef __cplusplus
}
#endif
