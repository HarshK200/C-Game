#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../main.h"
#include "win32_platform.h"
#include "win32_input.cpp"
#include "../game.cpp"
#include "renderer_d3d11.cpp"



// ================== Platform Provided Services ==================

// Prints a debug message to the windows console using
// OutputDebugString
void PlatformPrintDebug(const char* message) { OutputDebugString(message); }



// ====================== Internal functions ======================
namespace
{
    // Creates a window using win32 api and returns the HWND i.e.
    // WindowHandle If failed returns NULL
    HWND OpenWindow(HINSTANCE Instance)
    {
        WNDCLASS WindowClass = {};
        WindowClass.style = CS_OWNDC;
        WindowClass.lpfnWndProc = WindowInputCallback;
        WindowClass.hInstance = Instance;
        // TODO(harsh): add custom icon and cursor for the game
        // WindowClass.hIcon = ;
        // WindowClass.hCursor = ;
        WindowClass.lpszClassName = "isekaied_class";
        if (RegisterClass(&WindowClass) == 0)
        {
            OutputDebugString("\n[ERROR] Registering window failed\n");
            return NULL;
        }
        HWND WindowHandle =
            CreateWindowEx(NULL, WindowClass.lpszClassName, "isekaied",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                           CW_USEDEFAULT, 800, 600, 0, 0, Instance, 0);
        if (WindowHandle == NULL)
        {
            OutputDebugString("\n[ERROR] Unable to create window\n");
            return NULL;
        }

        return WindowHandle;
    }
} // namespace



// ================== Windows Application Entry Point ==================
int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCmd)
{
    // create app
    PlatformApp App = {};
    App.ShouldClose = false;

    // open window
    // TODO(harsh): handle allocation with an arena allocator
    App.Window = new PlatformWindow{};
    App.Window->Handle = OpenWindow(Instance);
    if (App.Window->Handle == NULL)
    {
        PlatformPrintDebug("[ERROR] OpenWindow failed. Exiting program...");
        return -1;
    }

    // Create Game and Renderer instance
    App.Game = GameCreateAndInit();
    if (App.Game == nullptr)
    {
        PlatformPrintDebug("[ERROR] Game init failed. Exiting program...");
        return -1;
    }
    App.Renderer = RendererCreateAndInit(App.Window);
    if (App.Renderer == nullptr)
    {
        PlatformPrintDebug("[ERROR] Renderer init failed. Exiting program...");
        return -1;
    }

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        // TODO(Create an input manager and pass that in instead of the whole
        // App instance):
        PollMessage(&App);

        GameUpdate(App.Game);
        RendererUpdate(App.Renderer, App.Game);
    }

    return App.ExitCode;
}
