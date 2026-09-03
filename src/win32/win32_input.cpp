#include <Windows.h>

#include "../main.h"
#include "win32_platform.h"

// ================== Input Layer Services Definitions ==================

// Handles the window messages and input
LRESULT CALLBACK InputWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{

    LRESULT result = 0;

    switch (message)
    {
        case WM_CREATE:
        {
            OutputDebugString("WM_CREATE\n");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
            break;
        }
        case WM_CLOSE:
        {
            OutputDebugString("WM_CLOSE\n");
            DestroyWindow(window);
            break;
        }
        case WM_DESTROY:
        {
            OutputDebugString("WM_DESTROY\n");
            PostQuitMessage(0);
            break;
        }
        default:
        {
            result = DefWindowProc(window, message, wparam, lparam);
            break;
        }
    }

    return result;
}

void InputPollMessage(PlatformApp* App)
{
    MSG Message;
    BOOL result = GetMessage(&Message, NULL, 0, 0);
    if (result == 0)
    {
        PlatformPrintDebug("[INFO] Close key pressed exiting");
        App->ShouldClose = true;
        App->ExitCode = 0;
        return;
    }
    if (result < 0)
    {
        PlatformPrintDebug("[ERROR] occured exiting");
        App->ShouldClose = true;
        App->ExitCode = -1;
        return;
    }
    TranslateMessage(&Message);
    DispatchMessage(&Message);
}
