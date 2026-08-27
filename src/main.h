#pragma once

// TODO(harsh): services that the platform layer provides
void PlatformPrintDebug(const char* message);

// NOTE(harsh): services that game provides to the platform layer
void GameInit();
void GameUpdate(); // pass delta_time, input

// NOTE(harsh): services that renderer provides
void RendererInit();
void RendererUpdate(); // pass game_state
