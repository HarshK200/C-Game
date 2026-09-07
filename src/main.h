#pragma once


// NOTE(harsh): platform layer services (types defined in platform.h)
struct PlatformApp;
struct PlatformWindow;
PlatformWindow* PlatformOpenWindow();


// TODO(harsh): input layer services implement a input manager which keep's
// track of an action map
struct InputManager;
void InputPollMessage(PlatformApp* App);


// NOTE(harsh): game layer services
struct Game;
Game* GameCreateAndInit();
void GameUpdate(Game* g); // TODO(harsh): pass delta_time, InputManager


// NOTE(harsh): rendering layer services
struct Renderer;
Renderer* RendererCreateAndInit(PlatformWindow* window);
void RendererUpdate(Renderer* r, Game* g, PlatformWindow* window);
