#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// the declarations
#include "src/main.h"

// the definitions
#include "src/game.cpp"
#include "src/win32/win32_platform.h"
#include "src/win32/win32_input.cpp"
#include "src/win32/renderer/renderer_d3d11.cpp"


// ================== Platform Layer Services Definitions ==================

// Prints a debug message to the windows console using
// OutputDebugString
void PlatformPrintDebug(const char* message)
{
    OutputDebugString(message);
    OutputDebugString("\n");
}

template <typename... T>
void PlatformPrintDebugF(const char* fstring, const T&... args)
{
    OutputDebugString(fstring);
    OutputDebugString("\n");

    // TODO(harsh): Implement the PrintDebugF functionallity
    // write the arena allocator so it can be used here for allocation
    //
    // sprintf(OutStr, fstring, args...);
    // OutputDebugString(OutStr);
}


// ====================== Internal functions ======================
namespace
{
    // Creates a window using win32 api and returns the HWND i.e.
    // WindowHandle If failed returns nullptr
    HWND OpenWindow(HINSTANCE Instance)
    {
        WNDCLASS window_class = {};
        window_class.style = CS_OWNDC;
        window_class.lpfnWndProc = InputWindowCallback;
        window_class.hInstance = Instance;
        // TODO(harsh): add custom icon and cursor for the game
        // WindowClass.hIcon = ;
        // WindowClass.hCursor = ;
        window_class.lpszClassName = "isekaied_class";
        if (RegisterClass(&window_class) == 0)
        {
            OutputDebugString("\n[ERROR] Registering window failed\n");
            return nullptr;
        }
        HWND window_handle = CreateWindowEx(
            NULL,
            window_class.lpszClassName,
            "isekaied",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            640, 360,
            0, 0,
            Instance,
            0);
        if (!window_handle)
        {
            OutputDebugString("\n[ERROR] Unable to create window\n");
            return nullptr;
        }
        return window_handle;
    }
} // namespace


// ================== Windows Application Entry Point ==================
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    // create app
    PlatformApp App = {};
    App.ShouldClose = false;

    // open window
    // TODO(harsh): handle allocation with an arena allocator
    App.Window = new PlatformWindow{};
    App.Window->Handle = OpenWindow(Instance);
    if (!App.Window->Handle)
    {
        PlatformPrintDebug("[ERROR] OpenWindow failed. Exiting program...");
        return -1;
    }

    // Create Game and Renderer instance
    App.Game = GameCreateAndInit();
    if (!App.Game)
    {
        PlatformPrintDebug("[ERROR] Game init failed. Exiting program...");
        return -1;
    }
    App.Renderer = RendererCreateAndInit(App.Window);
    if (!App.Renderer)
    {
        PlatformPrintDebug("[ERROR] Renderer init failed. Exiting program...");
        return -1;
    }

    // Main Update Loop
    while (App.ShouldClose == false)
    {
        // TODO(Create an input manager and pass that in instead of the whole
        // App instance):
        InputPollMessage(&App);

        GameUpdate(App.Game);
        RendererUpdate(App.Renderer, App.Game, App.Window);
    }

    return App.ExitCode;
}
