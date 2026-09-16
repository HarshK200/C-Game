#pragma once

// utils
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"
#include "src/renderer/render_data.cpp"


// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(AppMemory* memory);
int PlatformProcessInput(InputManager* input_manager);

// NOTE(harsh): game layer services
struct Game2d;
Game2d* GameCreateAndInit(AppMemory* memory);
void GameUpdate(AppMemory* memory, Game2d* g, InputManager* input_manager, RenderData* render_data); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
struct Renderer;
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window);
void RendererUpdate(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data);
