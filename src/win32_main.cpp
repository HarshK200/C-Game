#include <Windows.h>

LRESULT CALLBACK
MainWindowCallback(HWND Window,
                   UINT Message,
                   WPARAM WParam,
                   LPARAM LParam)
{

    LRESULT result = 0;

    switch (Message)
    {
        case WM_CREATE:
        {
            OutputDebugString("WM_CREATE\n");
            break;
        }
        case WM_CLOSE:
        {
            OutputDebugString("WM_CLOSE\n");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
            break;
        }
        case WM_DESTROY:
        {
            OutputDebugString("WM_DESTROY\n");
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

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        int ShowCmd)
{
    // Create a class for our window
    WNDCLASS WindowClass = {};

    WindowClass.style = CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon = ;
    // WindowClass.hCursor = ;
    WindowClass.lpszClassName = "Cpp_Game";

    if (RegisterClass(&WindowClass) == 0)
    {
        OutputDebugString("\n[ERROR] Registering window failed\n");
        return -1;
    }

    // TODO check if width height value is correct
    HWND WindowHandle = CreateWindowEx(
        NULL, WindowClass.lpszClassName, "Isakied Game",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance,
        0);

    if (WindowHandle == NULL)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return -1;
    }

    while (true)
    {
        MSG Message;
        BOOL result = GetMessage(&Message, NULL, 0, 0);

        if (result == 0)
        {
            OutputDebugString("Close key pressed exiting\n");
            return 0;
        }
        if (result < 0)
        {
            OutputDebugString("\n[ERROR] occured exiting\n");
            return -1;
        }

        // TODO understand wtf this thing does??
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return 0;
}
