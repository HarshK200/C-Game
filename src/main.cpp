// precompiled headers
#include "src/pch.h"

// utils
#include "src/utils/arena_allocator.h"

// Platform agnostic Declarations *ordered*
#include "main.h"
#include "renderer/render_data.cpp"
#include "src/utils/constants.h"


// =============================================================
//                          WINDOWS
// =============================================================
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
// Platform Layer Definitions
#include "src/win32/win32_platform.cpp"
#endif

// =============================================================
//                        GAME LAYER
// =============================================================
#include "src/game2d/game2d.cpp"
#include "src/game2d/camera2d.cpp"
#include "src/game2d/sprite2d.cpp"
#include "src/game2d/player.cpp"
#include "src/game2d/world.cpp"

// =============================================================
//                    D3D11 Renderer LAYER
// =============================================================
#include "src/renderer/render_data.cpp"
#include "src/renderer/d3d11/mesh.cpp"
#include "src/renderer/d3d11/texture_d3d11.cpp"
#include "src/renderer/d3d11/shader_d3d11.cpp"
#include "src/renderer/d3d11/renderer_d3d11.cpp"


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
    App.Renderer = RendererCreateAndInit(&App.Memory, App.Window);
    if (!App.Renderer)
    {
        LOG_ASSERT(false, "Renderer init failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }
    LOG_INFOF(
        "Size of render commands per frame: %d bytes for %d render commands",
        sizeof(RenderCommand) * MAX_RENDER_COMMANDS_PER_FRAME,
        MAX_RENDER_COMMANDS_PER_FRAME);

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        /*
            TODO(harsh): create a ActionMap struct in main.h file which will pass to as
            PlatformProcessInput(ActionMap* action_map) and it will update the input actions
            state for e.g. ActionMap[MOVE_LEFT] = PRESSED; or HELD or RELEASED or IDLE.

            Processing input
        */
        if (PlatformProcessInput() == WM_QUIT)
            App.ShouldClose = true;


        // Game update and render
        RenderData* render_data = CreateFrameRenderData(&App.Memory.TempAllocator);
        // TODO(harsh): pass action_map and delta time to GameUpate()
        GameUpdate(&App.Memory, App.Game, render_data);
        RendererUpdate(&App.Memory, App.Window, App.Renderer, render_data);


        // reset transient memory
        ArenaReset(&App.Memory.TempAllocator);
    }

program_exit:
    return App.ExitCode;
}
