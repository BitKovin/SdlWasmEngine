#pragma once

#ifdef __EMSCRIPTEN__

#include <fmod.hpp>
#include <fmod_common.h>
#include <fmod_errors.h>
#include <fmod_studio.hpp>

#else

#include <Fmod_desktop/fmod.hpp>
#include <Fmod_desktop/fmod_common.h>
#include <Fmod_desktop/fmod_errors.h>
#include <Fmod_desktop/fmod_studio.hpp>

#endif // __EMSCRIPTEN__