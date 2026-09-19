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
struct GameDLL;
#ifdef _WIN32
struct GameDLL
{
    HMODULE DLL;

    using CreateAndInitFn = Game* (*)(AppMemory*);
    using PhysicsUpdateFn = void (*)(AppMemory*, double, Game*, InputManager*);
    using UpdateFn = void (*)(AppMemory*, Game*, InputManager*);
    using QueueRenderFn = void (*)(AppMemory*, Game*, RenderData*, double);

    CreateAndInitFn CreateAndInit;
    PhysicsUpdateFn PhysicsUpdate;
    UpdateFn Update;
    QueueRenderFn QueueRender;

    /*
        Loads the dll and updates the function pointers.
        Returns 0 on success -1 on failure.
    */
    int Load(LPCSTR dll_path_relative)
    {
        this->DLL = LoadLibraryA(dll_path_relative);
        if (!this->DLL)
            return -1;

        this->CreateAndInit = (CreateAndInitFn)GetProcAddress(this->DLL, "GameCreateAndInit");
        this->PhysicsUpdate = (PhysicsUpdateFn)GetProcAddress(this->DLL, "GamePhysicsUpdate");
        this->Update = (UpdateFn)GetProcAddress(this->DLL, "GameUpdate");
        this->QueueRender = (QueueRenderFn)GetProcAddress(this->DLL, "GameQueueRender");

        if (!this->CreateAndInit ||
            !this->PhysicsUpdate ||
            !this->Update ||
            !this->QueueRender)
        {
            this->UnLoad();
            return -1;
        }


        return 0;
    }

    /*
        Unloads the currently loaded dll if any and sets the function
        pointers to nullptr.
    */
    bool UnLoad()
    {
        if (this->DLL == nullptr)
            return true;

        bool result = FreeLibrary(this->DLL);

        this->DLL = nullptr;
        this->CreateAndInit = nullptr;
        this->PhysicsUpdate = nullptr;
        this->Update = nullptr;
        this->QueueRender = nullptr;

        return result;
    }
};
#endif


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

    // Delta time stuff
    std::chrono::time_point<std::chrono::steady_clock> LastTimestamp;
    double DeltaTime;
    double Accumulator;

    PlatformWindow* Window;
    InputManager* InputManager;
    Game* Game;
    GameDLL GameDLL;
    Renderer* Renderer;
    RenderData* RenderData;

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

    // Load GameDLL
    if (App.GameDLL.Load("C:\\Users\\Harsh\\Desktop\\personal_dev\\cpp_game\\build\\debug\\game.dll") < 0)
    {
        LOG_ASSERT(false, "Falied to load Game DLL. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Open Platform Agnostic Window
    App.Window = PlatformOpenWindow(&App.Memory, App.InputManager);
    if (!App.Window)
    {
        LOG_ASSERT(false, "OpenWindow failed. Exiting program...");
        App.ExitCode = -1;
        goto program_exit;
    }

    // Create & Init Game instance
    App.Game = App.GameDLL.CreateAndInit(&App.Memory);
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
        if (PlatformProcessInput(App.InputManager) == 1)
            App.ShouldClose = true;

        // delta time calculation
        auto current_timestamp = std::chrono::steady_clock::now();
        double frame_time = std::chrono::duration<double>(current_timestamp - App.LastTimestamp).count();
        App.LastTimestamp = current_timestamp;
        App.Accumulator += frame_time;
        // TODO(harsh): put max frametime here

        // run physics simulation with fixed TimeStep and accumulate the rest
        while (App.Accumulator >= App.DeltaTime)
        {
            App.GameDLL.PhysicsUpdate(&App.Memory, App.DeltaTime, App.Game, App.InputManager);
            App.Accumulator -= App.DeltaTime;
        }
        double interpolation_alpha = App.Accumulator / App.DeltaTime;

        // per frame game update
        App.GameDLL.Update(&App.Memory, App.Game, App.InputManager);

        //  queue game entites render
        App.GameDLL.QueueRender(&App.Memory, App.Game, App.RenderData, interpolation_alpha);

        // render the frame
        RenderFrame(&App.Memory, App.Window, App.Renderer, App.RenderData);


        // cleanup
        ArenaReset(&App.Memory.TempAllocator);
    }

program_exit:
    return App.ExitCode;
}
