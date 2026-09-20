#include "src/pch.h"

// utils
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"


// ======================= SERVICES GLUE =========================
#include "src/input/input.h"
#include "src/renderer/render_data.h"
#include "src/utils/log.h"
#include <libloaderapi.h>


// ===================== PLATFORM SERVICES =======================
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(AppMemory* memory, InputManager* input_manager);
int PlatformProcessInput(InputManager* input_manager);
#ifdef _WIN32
#include "src/platform/win32/win32_platform.cpp"
#endif


// ======================= GAME SERVICES =========================
struct Game;
struct GameDLL
{
    void* DLL;
    const char* Name = "C:\\Users\\Harsh\\Desktop\\personal_dev\\cpp_game\\build\\debug\\game.dll";
    const char* LoadDLLName = "C:\\Users\\Harsh\\Desktop\\personal_dev\\cpp_game\\build\\debug\\game_load.dll";
    long long LastModifiedTimestamp;
};

using GameCreateAndInitFn = Game* (*)(AppMemory*);
using GamePhysicsUpdateFn = void (*)(AppMemory*, double, Game*, InputManager*);
using GameUpdateFn = void (*)(AppMemory*, Game*, InputManager*);
using GameQueueRenderFn = void (*)(AppMemory*, Game*, RenderData*, double);
static GameCreateAndInitFn GameCreateAndInit;
static GamePhysicsUpdateFn GamePhysicsUpdate;
static GameUpdateFn GameUpdate;
static GameQueueRenderFn GameQueueRender;

void ReloadGameDLL(ArenaAllocator* temp_allocator, GameDLL* game_dll);


// ====================== RENDERER SERVICES ======================
struct Renderer;
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window);
void RenderFrame(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data);
#ifdef _WIN32
#include "src/renderer/d3d11/renderer_d3d11.cpp"
#endif


// ===================== MAIN APP DEFINITION =====================
struct App
{
    bool ShouldClose;
    int ExitCode;

    // Delta time
    std::chrono::time_point<std::chrono::steady_clock> LastTimestamp;
    double DeltaTime;
    double Accumulator;

    // platform layer
    InputManager* InputManager;
    PlatformWindow* Window;

    // game
    GameDLL GameDLL;
    Game* Game;

    // renderer
    RenderData* RenderData;
    Renderer* Renderer;

    AppMemory Memory;
};


// ================== Application Entry Point ==================
int main()
{
    // create app *on the stack*
    App App = {};
    App.ShouldClose = false;
    App.Memory.PermanentAllocator = CreateArena(64 * MegaByte);
    App.Memory.TempAllocator = CreateArena(512 * MegaByte);
    App.InputManager = ArenaAlloc<InputManager>(&App.Memory.PermanentAllocator, sizeof(InputManager));

    // Reload Game DLL
    ReloadGameDLL(&App.Memory.TempAllocator, &App.GameDLL);

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
        // reload game dll
        ReloadGameDLL(&App.Memory.TempAllocator, &App.GameDLL);

        // re-create transient variables
        App.RenderData = CreateFrameRenderData(&App.Memory.TempAllocator);

        // input processing
        if (PlatformProcessInput(App.InputManager) == 1)
            App.ShouldClose = true;

        // delta time calculation
        auto current_timestamp = std::chrono::steady_clock::now();
        double frame_time = std::chrono::duration<double>(current_timestamp - App.LastTimestamp).count();
        if (frame_time > 0.25)
            frame_time = 0.25;
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


/*
    Reloads the game dll on game.dll rebuild.
    NOTE(harsh): This **WILL NOT WORK IF YOU HAVE A DEBUGGER ATTACHED**
*/
void ReloadGameDLL(ArenaAllocator* temp_allocator, GameDLL* game_dll)
{
    long long current_modified_timestamp = GameFileIO::GetLastModifiedTime(game_dll->Name);

    // check if the game.dll was updated
    if (current_modified_timestamp > game_dll->LastModifiedTimestamp)
    {
        // check if we've ever loaded the game.dll before, if we have unload it
        if (game_dll->DLL)
        {
            bool result = PlatformFreeDynamicLibrary(game_dll->DLL);
            LOG_ASSERT(result, "Failed to free Dynamic Library");

            game_dll->DLL = nullptr;
            LOG_INFOF("unloaded dynamic libarary %s", game_dll->Name);
        }

        int unsigned copy_file_retry_count = 10;

        // NOTE(harsh): cakez uses platform sleep here, probably so the code retries to load dll
        // again in 10 ms in case it was in use or maybe to avoid load fails while being written to?
        while (!GameFileIO::CopyFile(temp_allocator, game_dll->Name, game_dll->LoadDLLName) && copy_file_retry_count > 0)
        {
            LOG_WARN("Failed DLL file copy, retrying in 10ms...");
            PlatformSleep(10);
            copy_file_retry_count -= 1;
        }
        if (copy_file_retry_count <= 0)
        {
            LOG_ERROR("\nHot code reload failed, *MAKE SURE YOU DON'T HAVE A DEBUGGER ATTACHED* and try again\n");
            return;
        }

        // load the dll
        game_dll->DLL = PlatformLoadDynamicLibrary(game_dll->LoadDLLName);
        LOG_ASSERT(game_dll->DLL, "Failed to load load dll file at %s", game_dll->LoadDLLName);

        // update game function pointers
        GameCreateAndInit = (GameCreateAndInitFn)PlatformLoadDynamicFunction(game_dll->DLL, "GameCreateAndInit");
        GamePhysicsUpdate = (GamePhysicsUpdateFn)PlatformLoadDynamicFunction(game_dll->DLL, "GamePhysicsUpdate");
        GameUpdate = (GameUpdateFn)PlatformLoadDynamicFunction(game_dll->DLL, "GameUpdate");
        GameQueueRender = (GameQueueRenderFn)PlatformLoadDynamicFunction(game_dll->DLL, "GameQueueRender");

        // update last modified timestamp
        game_dll->LastModifiedTimestamp = current_modified_timestamp;
        LOG_OK("Hot Reloaded the game dll successfully");
    }
}
