// precompiled headers
#include "src/pch.h"

// utils
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"


// Platform agnostic Declarations *ordered*
#include "main.h"

// =============================================================
//                  PLATFORM LAYER DEFINITIONS
// =============================================================
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "src/win32/win32_platform.cpp"
#endif

// =============================================================
//                    GAME LAYER DEFINITIONS
// =============================================================
#include "src/game2d/game2d.cpp"
#include "src/game2d/camera2d.cpp"
#include "src/game2d/sprite2d.cpp"
#include "src/game2d/player.cpp"
#include "src/game2d/tilemap.cpp"

// =============================================================
//                  Renderer LAYER DEFINITIONS
// =============================================================
#include "src/renderer/render_data.cpp"
#ifdef _WIN32
#include "src/renderer/d3d11/mesh.cpp"
#include "src/renderer/d3d11/texture_d3d11.cpp"
#include "src/renderer/d3d11/shader_d3d11.cpp"
#include "src/renderer/d3d11/renderer_d3d11.cpp"
#endif


// ================== Application Entry Point ==================
int main()
{
    // create app *on the stack*
    App App = {};
    App.ShouldClose = false;
    App.Memory.PermanentAllocator = CreateArena(64 * MegaByte);
    App.Memory.TempAllocator = CreateArena(512 * MegaByte);
    App.InputManager = ArenaAlloc<InputManager>(&App.Memory.PermanentAllocator, sizeof(InputManager));

    // Open Platform Agnostic Window
    App.Window = PlatformOpenWindow(&App.Memory, App.InputManager);
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
    App.Renderer = RendererCreateAndInit(&App.Memory, App.Window);
    if (!App.Renderer)
    {
        LOG_ASSERT(false, "Renderer init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        // input processing
        if (PlatformProcessInput(App.InputManager) == WM_QUIT)
            App.ShouldClose = true;


        // Game update and render
        RenderData* render_data = CreateFrameRenderData(&App.Memory.TempAllocator);
        // TODO(harsh): pass delta time to GameUpate()
        GameUpdate(&App.Memory, App.Game, App.InputManager, render_data);
        RendererUpdate(&App.Memory, App.Window, App.Renderer, render_data);


        // cleanup
        ArenaReset(&App.Memory.TempAllocator);
    }

program_exit:
    return App.ExitCode;
}
