#pragma once

// TODO(harsh): input layer services implement a input manager which keep's
// track of an action map
struct InputManager;

// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
struct Renderer;
void PlatformPrintDebug(const char* message);


// NOTE(harsh): game layer services
struct Game;
Game* GameCreateAndInit();
void GameUpdate(Game* g); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
Renderer* RendererCreateAndInit(PlatformWindow* Window);
void RendererUpdate(Renderer* r, Game* g); // pass game_state
