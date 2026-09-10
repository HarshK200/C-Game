#pragma once

#include "src/utils/arena_allocator.h"


// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(AppMemory* memory);

// TODO(harsh): input layer services implement a input manager which keep's
// track of an action map
struct InputManager;


// NOTE(harsh): game layer services
struct Game2d;
Game2d* GameCreateAndInit(AppMemory* memory);
void GameUpdate(Game2d* g); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
struct Renderer;
Renderer* RendererCreateAndInit(PlatformWindow* window, AppMemory* memory);
void RendererUpdate(Renderer* r, Game2d* g, PlatformWindow* window);
