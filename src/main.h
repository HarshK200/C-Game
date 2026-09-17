#pragma once

// utils
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"
#include "src/renderer/render_data.cpp"
#include <chrono>


// =============================================================
//                  PLATFORM SERVICES DECLARATIONS
// =============================================================
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(AppMemory* memory, InputManager* input_manager);
int PlatformProcessInput(InputManager* input_manager);


// =============================================================
//                    GAME SERVICES DECLARATIONS
// =============================================================
struct Game;
Game* GameCreateAndInit(AppMemory* memory);
void GamePhysicsUpdate(AppMemory* memory, double delta_time, Game* g, InputManager* input_manager);
void GameUpdate(AppMemory* memory, Game* g, InputManager* input_manager);
void GameQueueRender(AppMemory* memory, Game* g, RenderData* render_data, double interpolation_alpha);


// =============================================================
//                  RENDERER SERVICES DECLARATIONS
// =============================================================
struct Renderer;
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window);
void RenderFrame(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data);


// =============================================================
//                          MAIN APP
// =============================================================
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
