#pragma once

#include "src/utils/arena_allocator.h"


// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
struct PlatformInputManager; // TODO(harsh): figure out the in between cross platform action map
PlatformWindow* PlatformOpenWindow(AppMemory* memory);
int PlatformProcessInput();


// NOTE(harsh): game layer services
struct Game2d;
Game2d* GameCreateAndInit(AppMemory* memory);
void GameUpdate(Game2d* g); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
struct Renderer;
Renderer* RendererCreateAndInit(PlatformWindow* window, AppMemory* memory);
void RendererUpdate(Renderer* r, Game2d* g, PlatformWindow* window);
