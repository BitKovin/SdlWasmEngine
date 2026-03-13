/*
 * RmlUi SDL + bgfx Backend
 *
 * Drop-in replacement for RmlUi_Backend_SDL_GL3.cpp.
 * Uses bgfx for rendering instead of raw OpenGL 3.
 */

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_BGFX.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Profiling.h>

#include <includedLibraries/stb_image.h>
#include <PlatformMains/PlatformWindowData.h>
#include <Profiling/ResourceStatistics.hpp>

#include <bgfx/bgfx.h>

/**
   Custom render interface for the SDL/BGFX backend.

   Overloads the bgfx render interface to load textures through stb_image,
   exactly as the GL3 SDL backend does.
*/
class RenderInterface_BGFX_SDL : public RenderInterface_BGFX
{
public:
    RenderInterface_BGFX_SDL() {}

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                    const Rml::String& source) override
    {
        // Delegate to base class — it already uses stb_image + premultiplied alpha,
        // matching the GL3 SDL backend behaviour identically.
        return RenderInterface_BGFX::LoadTexture(texture_dimensions, source);
    }
};

/**
   Global data used by this backend.
   Lifetime governed by Backend::Initialize() / Backend::Shutdown().
*/
struct BackendData
{
    SystemInterface_SDL       system_interface;
    RenderInterface_BGFX_SDL  render_interface;

    SDL_Window*  window    = nullptr;
    bool         running   = true;
};
static Rml::UniquePtr<BackendData> data;

// ─────────────────────────────────────────────────────────────────────────────

bool Backend::Initialize(const char* /*window_name*/, int width, int height, bool /*allow_resize*/)
{
    RMLUI_ASSERT(!data);

    // bgfx is assumed to be already initialised by the engine (EngineMain).
    // SDL window is assumed to be already created and stored in PlatformWindowData.

    data = Rml::MakeUnique<BackendData>();

    if (!data->render_interface)
    {
        data.reset();
        Rml::Log::Message(Rml::Log::LT_ERROR, "BGFX: Failed to initialise render interface (shaders not found?)");
        return false;
    }

    data->window = PlatformWindowData::window;

    data->system_interface.SetWindow(PlatformWindowData::window);
    data->render_interface.SetViewport(width, height);

    return true;
}

void Backend::Shutdown()
{
    RMLUI_ASSERT(data);

    // We do NOT destroy the SDL window or call SDL_Quit here —
    // the engine owns both the window and the bgfx context.

    data.reset();
}

Rml::SystemInterface* Backend::GetSystemInterface()
{
    RMLUI_ASSERT(data);
    return &data->system_interface;
}

Rml::RenderInterface* Backend::GetRenderInterface()
{
    RMLUI_ASSERT(data);
    return &data->render_interface;
}

bool Backend::ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback, bool power_save)
{
    RMLUI_ASSERT(data && context);

#if SDL_MAJOR_VERSION >= 3
#define RMLSDL_WINDOW_EVENTS_BEGIN
#define RMLSDL_WINDOW_EVENTS_END
    auto GetKey = [](const SDL_Event& event) { return event.key.key; };
    auto GetDisplayScale = []() { return SDL_GetWindowDisplayScale(data->window); };
    constexpr auto event_quit              = SDL_EVENT_QUIT;
    constexpr auto event_key_down          = SDL_EVENT_KEY_DOWN;
    constexpr auto event_window_size_changed = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    bool has_event = false;
#else
#define RMLSDL_WINDOW_EVENTS_BEGIN  \
    case SDL_WINDOWEVENT:           \
    {                               \
        switch (ev.window.event)    \
        {
#define RMLSDL_WINDOW_EVENTS_END    \
        }                           \
        }                           \
        break;
    auto GetKey = [](const SDL_Event& event) { return event.key.keysym.sym; };
    auto GetDisplayScale = []() { return 1.f; };
    constexpr auto event_quit              = SDL_QUIT;
    constexpr auto event_key_down          = SDL_KEYDOWN;
    constexpr auto event_window_size_changed = SDL_WINDOWEVENT_SIZE_CHANGED;
    int has_event = 0;
#endif

    bool result = data->running;
    data->running = true;

    SDL_Event ev;
    if (power_save)
        has_event = SDL_WaitEventTimeout(&ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0) * 1000));
    else
        has_event = SDL_PollEvent(&ev);

    while (has_event)
    {
        bool propagate_event = true;
        switch (ev.type)
        {
        case event_quit:
            propagate_event = false;
            result = false;
            break;

        case event_key_down:
        {
            propagate_event = false;
            const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(ev));
            const int key_modifier = RmlSDL::GetKeyModifierState();
            const float native_dp_ratio = GetDisplayScale();

            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true))
                break;
            if (!RmlSDL::InputEventHandler(context, data->window, ev))
                break;
            if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false))
                break;
        }
        break;

        RMLSDL_WINDOW_EVENTS_BEGIN

        case event_window_size_changed:
        {
            Rml::Vector2i dimensions = { ev.window.data1, ev.window.data2 };
            data->render_interface.SetViewport(dimensions.x, dimensions.y);
        }
        break;

        RMLSDL_WINDOW_EVENTS_END

        default:
            break;
        }

        if (propagate_event)
            RmlSDL::InputEventHandler(context, data->window, ev);

        has_event = SDL_PollEvent(&ev);
    }

    return result;
}

void Backend::RequestExit()
{
    RMLUI_ASSERT(data);
    data->running = false;
}

void Backend::BeginFrame()
{
    RMLUI_ASSERT(data);
    data->render_interface.Clear();
    data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
    RMLUI_ASSERT(data);
    data->render_interface.EndFrame();

    // bgfx::frame() is called by the engine's main loop, not here.
    // If your engine requires it, uncomment:
    // bgfx::frame();

    RMLUI_FrameMark;
}
