// Declarations ordered
#include "main.h"


#ifdef _WIN32

// Definitions *unordered*
#include "src/win32/win32_platform.cpp"
#include "src/game/game.cpp"
#include "src/win32/win32_input.cpp"
#include "src/win32/renderer/draw/draw.cpp"
#include "src/win32/renderer/mesh.cpp"
#include "src/win32/renderer/texture.cpp"
#include "src/win32/renderer/shader_d3d11.cpp"
#include "src/win32/renderer/renderer_d3d11.cpp"

#endif


// ================== Application Entry Point ==================
int main()
{
    // create app *on the stack*
    PlatformApp App = {};
    App.ShouldClose = false;
    // TODO(harsh): maybe? create one allocator per system like: game_allocator, renderer_allocator, input_allocator
    App.PermanentAllocator = CreateArena(64 * MegaByte);
    App.TempAllocator = CreateArena(512 * MegaByte);

    // Open Platform agnostic Window
    App.Window = PlatformOpenWindow(&App.PermanentAllocator);
    if (!App.Window)
    {
        LOG_ASSERT(false, "OpenWindow failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create & Init Game instance
    App.Game = GameCreateAndInit(&App.PermanentAllocator);
    if (!App.Game)
    {
        LOG_ASSERT(false, "Game init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create & Init Renderer instance
    App.Renderer = RendererCreateAndInit(App.Window, &App.PermanentAllocator, &App.TempAllocator);
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
