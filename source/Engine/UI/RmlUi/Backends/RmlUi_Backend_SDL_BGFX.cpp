#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_BGFX.h"
#include <RmlUi/Core/Core.h>
#include <PlatformMains/PlatformWindowData.h>

struct BackendData {
    SystemInterface_SDL   system_interface;
    RenderInterface_BGFX* render_interface = nullptr;
    bool running = true;
};
static Rml::UniquePtr<BackendData> data;

bool Backend::Initialize(const char* /*window_name*/, int width, int height, bool /*allow_resize*/)
{
    if (!data) data = Rml::MakeUnique<BackendData>();

    data->render_interface = new RenderInterface_BGFX();
    data->render_interface->SetViewport(width, height);

    data->system_interface.SetWindow(PlatformWindowData::window);

    return true;
}

void Backend::Shutdown()
{
    delete data->render_interface;
    data.reset();
}

Rml::SystemInterface* Backend::GetSystemInterface() { return &data->system_interface; }
Rml::RenderInterface* Backend::GetRenderInterface() { return data->render_interface; }

void Backend::BeginFrame()
{
    // BeginFrame() must run first — it assigns m_uiView and sets
    // ViewMode::Sequential. Clear() uses m_uiView so must come after.
    data->render_interface->BeginFrame();
    data->render_interface->Clear();
}

void Backend::PresentFrame()
{
    data->render_interface->EndFrame();
}