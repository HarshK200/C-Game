#include "main.h"

#ifdef _WIN32
#include "src/win32/win32_platform.cpp"
#endif


// ================== Windows Application Entry Point ==================
int main()
{
    // create app
    PlatformApp App = {};
    App.ShouldClose = false;

    // open window
    // TODO(harsh): handle allocation with an arena allocator
    App.Window = PlatformOpenWindow();
    if (!App.Window->Handle)
    {
        LOG_ASSERT(false, "OpenWindow failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create Game and Renderer instance
    App.Game = GameCreateAndInit();
    if (!App.Game)
    {
        LOG_ASSERT(false, "Game init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }
    App.Renderer = RendererCreateAndInit(App.Window);
    if (!App.Renderer)
    {
        LOG_ASSERT(false, "Renderer init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        // TODO(Create an input manager and pass that in instead of the whole App instance):
        InputPollMessage(&App);

        GameUpdate(App.Game);
        RendererUpdate(App.Renderer, App.Game, App.Window);
    }

program_exit:
    return App.ExitCode;
}
