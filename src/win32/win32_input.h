#pragma once

#include "src/win32/win32_platform.h"

// Struct Definitions Only
struct InputManager
{
};


LRESULT CALLBACK InputWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void InputPollMessage(PlatformApp* app);
