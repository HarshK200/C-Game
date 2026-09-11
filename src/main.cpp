// precompiled headers
#include "src/pch.h"

// Declarations *ordered*
#include "main.h"


#ifdef _WIN32

// windows specific defines
#define WIN32_LEAN_AND_MEAN

// Definitions *unordered*
#include "src/win32/win32_platform.cpp"
#include "src/game2d/game2d.cpp"
#include "src/game2d/camera2d.cpp"
#include "src/game2d/player/player.cpp"
#include "src/win32/win32_input.cpp"
#include "src/win32/renderer/mesh.cpp"
#include "src/win32/renderer/texture_d3d11.cpp"
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
    App.Memory.PermanentAllocator = CreateArena(64 * MegaByte);
    App.Memory.TempAllocator = CreateArena(512 * MegaByte);

    // Open Platform agnostic Window
    App.Window = PlatformOpenWindow(&App.Memory);
    if (!App.Window)
    {
        LOG_ASSERT(false, "OpenWindow failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create & Init Game instance
    App.Game = GameCreateAndInit(&App.Memory);
    if (!App.Game)
    {
        LOG_ASSERT(false, "Game init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create & Init Renderer instance
    App.Renderer = RendererCreateAndInit(App.Window, &App.Memory);
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
        if (ProcessInput() == WM_QUIT)
            App.ShouldClose = true;

        GameUpdate(App.Game);
        RendererUpdate(App.Renderer, App.Game, App.Window);
    }

program_exit:
    return App.ExitCode;
}
