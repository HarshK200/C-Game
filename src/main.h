#pragma once

// TODO(harsh): services that the platform layer provides
struct PlatformApp;
struct PlatformWindow;

void PlatformPrintDebug(const char* message);

// NOTE(harsh): services that game provides to the platform layer
void GameInit();
void GameUpdate(); // pass delta_time, input

// NOTE(harsh): services that renderer provides
void RendererInit(PlatformWindow* Window);
void RendererUpdate(); // pass game_state
