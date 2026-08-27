#include "game.cpp"
#include "win32_renderer_d3d11.cpp"
#include <Windows.h>

LRESULT CALLBACK MainWindowCallback(HWND Window,
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

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCmd)
{

    // ====================== Opening a window ======================
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
    HWND WindowHandle = CreateWindowEx(
        NULL, WindowClass.lpszClassName, "Isakied Game",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
        CW_USEDEFAULT, 800, 600, 0, 0, Instance, 0);
    if (WindowHandle == NULL)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return -1;
    }

    // =================== Init ====================
    GameInit();
    RendererInit();

    // ======= Update loop =======
    while (true)
    {
        // ======= Handle input =======
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

        // TODO(harsh): understand wtf this thing does??
        TranslateMessage(&Message);
        DispatchMessage(&Message);

        // ======= Game Updates =======
        GameUpdate();

        // ======= Renderer Updates =======
        RendererUpdate();
    }

    return 0;
}

// Prints a debug message to the windows console using
// OutputDebugString
void PlatformPrintDebug(const char* message)
{
    OutputDebugString(message);
}
