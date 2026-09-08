#pragma once
#include "src/utils/arena_allocator.h"


// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow(ArenaAllocator* permanent_allocator);


// TODO(harsh): input layer services implement a input manager which keep's
// track of an action map
struct InputManager;
void InputPollMessage(PlatformApp* app);


// NOTE(harsh): game layer services
struct Game;
Game* GameCreateAndInit(ArenaAllocator* persistent_allocator);
void GameUpdate(Game* g); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
struct Renderer;
Renderer* RendererCreateAndInit(PlatformWindow* window, ArenaAllocator* permanent_allocator, ArenaAllocator* temp_allocator);
void RendererUpdate(Renderer* r, Game* g, PlatformWindow* window);
