#pragma once

#include <Windows.h>

#include "src/main.h"
#include "src/utils/arena_allocator.h"


// ================== Platform Provided Structs Function Definitions ==================
struct PlatformWindow
{
    HWND Handle;
};
struct PlatformApp
{
    bool ShouldClose;
    int ExitCode;

    PlatformWindow* Window;
    Game2d* Game;
    Renderer* Renderer; // D3D11

    AppMemory Memory;
};
struct PlatformInputManager
{
};
