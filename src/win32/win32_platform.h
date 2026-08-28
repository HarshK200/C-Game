#pragma once
#include <Windows.h>

struct PlatformWindow
{
    HWND Handle;
};

struct PlatformApp
{
    PlatformWindow Window;
};
