#pragma once

#include <Windows.h>

// Struct Definitions Only
struct InputManager
{
};


LRESULT CALLBACK WindowMessageCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
int ProcessInput();
