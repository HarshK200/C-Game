#include <chrono>

// utils
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"

// layers glue files
#include "src/input/input.h"
#include "src/renderer/render_data.h"


// ================= PLATFORM SERVICES DECLARATIONS ==============
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(AppMemory* memory, InputManager* input_manager);
int PlatformProcessInput(InputManager* input_manager);


// =================== GAME SERVICES DECLARATIONS ================
struct Game;
Game* GameCreateAndInit(AppMemory* memory);
void GamePhysicsUpdate(AppMemory* memory, double delta_time, Game* g, InputManager* input_manager);
void GameUpdate(AppMemory* memory, Game* g, InputManager* input_manager);
void GameQueueRender(AppMemory* memory, Game* g, RenderData* render_data, double interpolation_alpha);


// ================= RENDERER SERVICES DECLARATIONS ==============
struct Renderer;
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window);
void RenderFrame(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data);


// ===================== MAIN APP DEFINITION =====================
struct App
{
    bool ShouldClose;
    int ExitCode;

    // Delta time stuff
    std::chrono::time_point<std::chrono::steady_clock> LastTimestamp;
    double DeltaTime;
    double Accumulator;

    PlatformWindow* Window;
    InputManager* InputManager;
    Game* Game;
    Renderer* Renderer;
    RenderData* RenderData;

    AppMemory Memory;
};


// ================= PLATFORM LAYER DEFINITIONS ==================
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "src/platform/win32/win32_platform.cpp"
#endif

// =================== GAME LAYER DEFINITIONS ====================
#include "src/game2d/game2d.cpp"

// ================= Renderer LAYER DEFINITIONS ==================
#ifdef _WIN32
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
