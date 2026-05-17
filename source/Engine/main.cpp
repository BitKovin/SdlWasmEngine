#ifndef ENGINE_AS_LIBRARY 



#if defined(__EMSCRIPTEN__)
#include "PlatformMains/emscripten_main.hpp"
#elif defined(PLATFORM_GDK)
#include "PlatformMains/gdk_main.hpp"
#elif defined(_WIN32) || defined(WIN32)
#include "PlatformMains/windows_main.hpp"
#elif defined(__linux__)
#include "PlatformMains/linux_main.hpp"
#else
#error "Unknown platform: cannot include the proper main()"
#endif



#endif // ENGINE_AS_LIBRARY