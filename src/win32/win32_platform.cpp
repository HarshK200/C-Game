#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// declarations
#include "src/main.h"

// constants
#include "src/utils/constants.h"

// definitions
#include "src/game.cpp"
#include "src/win32/win32_input.cpp"
#include "src/win32/renderer/mesh.cpp"
#include "src/win32/renderer/shader_d3d11.cpp"
#include "src/win32/renderer/renderer_d3d11.cpp"


// ================== Platform Layer Services Definitions ==================

/*
    Creates a window using win32 api and returns the PlatformWindow* on success,
    otherwise returns nullptr on failure
*/
PlatformWindow* PlatformOpenWindow()
{
    HINSTANCE instance = GetModuleHandle(0);
    PlatformWindow* window = new PlatformWindow{};

    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = InputWindowCallback;
    window_class.hInstance = instance;
    // TODO(harsh): add custom icon and cursor for the game
    window_class.hIcon = LoadIcon(instance, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = "isekaied_class";
    if (RegisterClass(&window_class) == 0)
    {
        OutputDebugString("\n[ERROR] Registering window failed\n");
        return nullptr;
    }
    window->Handle = CreateWindowEx(
        NULL,
        window_class.lpszClassName,
        "isekaied",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        DEFAULT_WINDOW_RESOLUTION.x,
        DEFAULT_WINDOW_RESOLUTION.y,
        NULL,
        NULL,
        instance,
        NULL);
    if (!window->Handle)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
    }

    return window;
}
