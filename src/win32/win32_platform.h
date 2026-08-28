#pragma once
#include <Windows.h>

// NOTE(harsh): This file is ONLY for the type definitions,
// Don't put any function signature declarations here that are Platform Agnostic
// They should live in main.h and implemented in win32_platform.cpp

struct PlatformWindow
{
    HWND Handle;
};

struct PlatformApp
{
    PlatformWindow Window;
};
