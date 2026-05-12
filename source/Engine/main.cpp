#include <SDL2/SDL.h> // SDL defines __GDK__ here if _GAMING_XXX is present

#if defined(__EMSCRIPTEN__)
#include "PlatformMains/emscripten_main.hpp"
#elif defined(__GDK__) || defined (_GAMING_DESKTOP)
#include "PlatformMains/gdk_main.hpp"
#elif defined(_WIN32) || defined(WIN32)
#include "PlatformMains/windows_main.hpp"
#elif defined(__linux__)
#include "PlatformMains/linux_main.hpp"
#else
#error "Unknown platform: cannot include the proper main()"
#endif