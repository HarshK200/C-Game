#include <Windows.h>
#include <winuser.h>

#include "src/win32/win32_input.h"

// ================== Internal functions only to be used by win32_platform ==================

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

/*
    Proccess all the input events in the Window Message Queue
    If message is WM_QUIT returns WM_QUIT
    otherwise returns 0 on successful finish

    NOTE(harsh): for now this function does not fail.
    if it fails means shit really went wrong

    TODO(harsh): see if PeekMessage can fail or not? if it can handle that)
*/
int ProcessInput()
{
    MSG message;
    int result = 0;

    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);

        if (message.message == WM_QUIT)
            result = WM_QUIT;
    }

    return result;
}
