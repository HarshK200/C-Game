// precompiled headers
#include "src/pch.h"


// Declarations *ordered*
#include "main.h"


// =============================================================
//                          WINDOWS
// =============================================================
#ifdef _WIN32
// windows specific defines
#define WIN32_LEAN_AND_MEAN

// Platform Layer Definitions
#include "src/win32/win32_platform.cpp"

// Game Layer Definitions
#include "src/game2d/game2d.cpp"
#include "src/game2d/camera2d.cpp"
#include "src/game2d/player/player.cpp"

// Renderer Layer Definitions
#include "src/renderer/d3d11/mesh.cpp"
#include "src/renderer/d3d11/texture_d3d11.cpp"
#include "src/renderer/d3d11/shader_d3d11.cpp"
#include "src/renderer/d3d11/renderer_d3d11.cpp"
#endif


// ================== Application Entry Point ==================
int main()
{
    // create app *on the stack*
    PlatformApp App = {};
    App.ShouldClose = false;
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
        if (PlatformProcessInput() == WM_QUIT)
            App.ShouldClose = true;

        GameUpdate(App.Game);
        RendererUpdate(App.Renderer, App.Game, App.Window);
    }

program_exit:
    return App.ExitCode;
}
