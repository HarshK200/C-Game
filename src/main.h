#pragma once

// utils
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"
#include "src/renderer/render_data.cpp"


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
// TODO(harsh): pass delta_time
void GameUpdate(AppMemory* memory, Game* g, InputManager* input_manager, RenderData* render_data);


// =============================================================
//                  RENDERER SERVICES DECLARATIONS
// =============================================================
struct Renderer;
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window);
void RendererUpdate(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data);


// =============================================================
//                          MAIN APP
// =============================================================
struct App
{
    bool ShouldClose;
    int ExitCode;

    PlatformWindow* Window;
    InputManager* InputManager;
    Game* Game;
    Renderer* Renderer;

    AppMemory Memory;
};
