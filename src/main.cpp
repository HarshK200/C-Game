// precompiled headers
#include "src/pch.h"
#include <chrono>

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

    // Record Initial delta time
    App.LastTimestamp = std::chrono::steady_clock::now();
    App.DeltaTime = FIXED_PHYSICS_DELTA_TIME; // this is 0.01 sec i.e. 10ms per physics update
    App.Accumulator = 0.0f;

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        // re-create transient variables
        App.RenderData = CreateFrameRenderData(&App.Memory.TempAllocator);


        // input processing
        if (PlatformProcessInput(App.InputManager) == WM_QUIT)
            App.ShouldClose = true;

        // delta time calculation
        auto current_timestamp = std::chrono::steady_clock::now();
        double frame_time = std::chrono::duration<double>(current_timestamp - App.LastTimestamp).count();
        App.LastTimestamp = current_timestamp;
        App.Accumulator += frame_time;

        // run physics simulation with fixed TimeStep and accumulate the rest
        while (App.Accumulator >= App.DeltaTime)
        {
            GamePhysicsUpdate(&App.Memory, App.DeltaTime, App.Game, App.InputManager);
            App.Accumulator -= App.DeltaTime;
        }
        double interpolation_alpha = App.Accumulator / App.DeltaTime;

        // per frame game update
        GameUpdate(&App.Memory, App.Game, App.InputManager);

        //  queue game entites render
        GameQueueRender(&App.Memory, App.Game, App.RenderData, interpolation_alpha);

        // render the frame
        RenderFrame(&App.Memory, App.Window, App.Renderer, App.RenderData);


        // cleanup
        ArenaReset(&App.Memory.TempAllocator);
    }

program_exit:
    return App.ExitCode;
}
