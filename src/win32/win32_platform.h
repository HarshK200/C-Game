// NOTE(harsh): This file is ONLY for the type definitions,
// Don't put any function signature declarations here that are either internal
// or exposed by platform layer They should live in main.h and implemented in
// win32_platform.cpp

#pragma once
#include <Windows.h>
#include "../main.h"


struct PlatformWindow
{
    HWND Handle;
};


struct PlatformApp
{
    bool ShouldClose;
    int ExitCode;

    PlatformWindow* Window;
    Game* Game;
    Renderer* Renderer; // D3D11
};
