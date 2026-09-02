#include <Windows.h>

#include "../main.h"
#include "win32_platform.h"

// Handles the window messages and input
LRESULT CALLBACK WindowInputCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{

    LRESULT result = 0;

    switch (Message)
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
            DestroyWindow(Window);
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
            result = DefWindowProc(Window, Message, WParam, LParam);
            break;
        }
    }

    return result;
}

void PollMessage(PlatformApp* App)
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
