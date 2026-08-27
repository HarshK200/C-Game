#pragma once

// NOTE(harsh): services taht the platform layer provides
void PlatformPrintDebug(const char* message);

// NOTE(harsh): services that game provides to the platform layer
void GameInit();

// NOTE(harsh): services that renderer provides
void RendererInit();

#include "game.cpp"
#include "renderer.cpp"
