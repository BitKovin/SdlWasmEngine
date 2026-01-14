/**
@defgroup detour Detour

Members in this module are wrappers around the standard math library
*/

#ifndef DETOURMATH_H
#define DETOURMATH_H

#include <cmath>

inline float dtMathFabsf(float x) { return std::fabsf(x); }
inline float dtMathSqrtf(float x) { return std::sqrtf(x); }
inline float dtMathFloorf(float x) { return std::floorf(x); }
inline float dtMathCeilf(float x) { return std::ceilf(x); }
inline float dtMathCosf(float x) { return std::cosf(x); }
inline float dtMathSinf(float x) { return std::sinf(x); }
inline float dtMathAtan2f(float y, float x) { return std::atan2f(y, x); }
inline bool dtMathIsfinite(float x) { return std::isfinite(x); }

#endif
