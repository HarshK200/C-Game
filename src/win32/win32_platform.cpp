#pragma once

#include <Windows.h>

// utils
#include "src/utils/log.h"
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"

#include "src/main.h"
#include "src/input/input.h"


// =================================================================================
//                      PLATFORM SERVICES STRUCTS DEFINITIONS
// =================================================================================
struct PlatformWindow
{
    HWND Handle;
};
struct PlatformApp
{
    bool ShouldClose;
    int ExitCode;

    PlatformWindow* Window;
    InputManager* InputManager;
    Game2d* Game;
    Renderer* Renderer; // D3D11

    AppMemory Memory;
};


// =================================================================================
//                              INTERNAL FUNCTIONS
// =================================================================================
/*
    Windows message callback. This function gets called everytime windows Dispatch's a message
    i.e. everytime DispatchMessage() is called.
*/
LRESULT CALLBACK WindowMessageCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{

    LRESULT result = 0;

    switch (message)
    {
        case WM_CREATE:
        {
            LOG_INFO("WM_CREATE");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            LOG_INFO("WM_ACTIVATEAPP");
            break;
        }
        case WM_CLOSE:
        {
            LOG_INFO("WM_CLOSE");
            DestroyWindow(window);
            break;
        }
        case WM_DESTROY:
        {
            LOG_INFO("WM_DESTROY");
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
            // TODO(harsh):implement viewport resizing in the renderer,
            // Here set the in INPUT_MANAGER input action window resize
            LOG_INFO("WM_SIZE");
            break;
        default:
        {
            result = DefWindowProc(window, message, wparam, lparam);
            break;
        }
    }

    return result;
}


// =================================================================================
//                      PLATFORM SERVICES FUNCTION DEFINITIONS
// =================================================================================

/*
    Creates a window using win32 api and returns the PlatformWindow* on success,
    otherwise returns nullptr on failure
*/
PlatformWindow* PlatformOpenWindow(AppMemory* memory)
{
    PlatformWindow* window = ArenaAlloc<PlatformWindow>(&memory->PermanentAllocator, sizeof(PlatformWindow));

    HINSTANCE instance = GetModuleHandle(0);
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = WindowMessageCallback;
    window_class.hInstance = instance;
    // TODO(harsh): add custom icon and cursor for the game
    window_class.hIcon = LoadIcon(instance, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = "isekaied_class";
    if (RegisterClass(&window_class) == 0)
    {
        LOG_ERROR("Registering window failed.");
        return nullptr;
    }

    // NOTE(harsh): VERY IMPORTANT!! Calculate the outer window size based on the
    // window style we are using i.e. this will account for the drawable window rect +
    // style like border, menu, etc...
    DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT window_rect = {
        0,
        0,
        (LONG)DEFAULT_WINDOW_RESOLUTION.x,
        (LONG)DEFAULT_WINDOW_RESOLUTION.y,
    };
    AdjustWindowRect(&window_rect, window_style, FALSE);
    int outer_width = window_rect.right - window_rect.left;
    int outer_height = window_rect.bottom - window_rect.top;

    window->Handle = CreateWindowEx(
        NULL,
        window_class.lpszClassName,
        "isekaied",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // use WS_POPUP instane of WS_OVERLAPPEDWINDOW for a boderless window
        CW_USEDEFAULT, CW_USEDEFAULT,
        outer_width, outer_height,
        NULL,
        NULL,
        instance,
        NULL);
    if (!window->Handle)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return nullptr;
    }

    return window;
}

/*
    Proccess all the input events in the Window Message Queue
    If message is WM_QUIT returns WM_QUIT
    otherwise returns 0 on successful finish
*/
int PlatformProcessInput(InputManager* input_manager)
{
    MSG message;
    int result = 0;

    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
    {
        // TODO(harsh): maybe add error handling if translate and dispatch fails??
        TranslateMessage(&message);
        DispatchMessage(&message);

        if (message.message == WM_QUIT)
            result = WM_QUIT;
    }

    return result;
}
