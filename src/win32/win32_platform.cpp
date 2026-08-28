#include <Windows.h>

#include "../main.h"
#include "win32_platform.h"
#include "../game.cpp"
#include "win32_platform_renderer_d3d11.cpp"

// ===== Internal declerations =====
LRESULT CALLBACK WindowInputCallback(HWND Window,
                                     UINT Message,
                                     WPARAM WParam,
                                     LPARAM LParam);
HWND OpenWindow(HINSTANCE Instance);
void SetupGraphicsApiAndSwapchain(HWND WindowHandle);

// ===== Windows Application Entry Point =====
int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCmd)
{
    PlatformApp App = {};
    App.Window.Handle = OpenWindow(Instance);

    // ===== Init =====
    GameInit();
    RendererInit(App.Window);

    // ===== Update Loop =====
    while (true)
    {
        // TODO(harsh): move this to a handle input function, which
        // updates a the input map
        MSG Message;
        BOOL result = GetMessage(&Message, NULL, 0, 0);
        if (result == 0)
        {
            OutputDebugString("[INFO] Close key pressed exiting\n");
            return 0;
        }
        if (result < 0)
        {
            OutputDebugString("\n[ERROR] occured exiting\n");
            return -1;
        }
        TranslateMessage(&Message);
        DispatchMessage(&Message);

        // NOTE(harsh): platform agnostic update code
        GameUpdate();
        RendererUpdate();
    }

    return 0;
}

// ===== Internal definitions =====
LRESULT CALLBACK WindowInputCallback(HWND Window,
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
    WindowClass.lpszClassName = "Cpp_Game";
    if (RegisterClass(&WindowClass) == 0)
    {
        OutputDebugString("\n[ERROR] Registering window failed\n");
        return NULL;
    }
    HWND WindowHandle = CreateWindowEx(
        NULL, WindowClass.lpszClassName, "Isakied Game",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
        CW_USEDEFAULT, 800, 600, 0, 0, Instance, 0);
    if (WindowHandle == NULL)
    {
        OutputDebugString("\n[ERROR] Unable to create window\n");
        return NULL;
    }

    return WindowHandle;
}

void SetupGraphicsApiAndSwapchain(HWND WindowHandle)
{

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 640;
    sd.BufferDesc.Height = 480;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = WindowHandle;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = true;
    D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;

    // return values from create device and swapchain
    D3D_FEATURE_LEVEL FeatureLevel;
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;

    // create the device and swapchain
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevels, 1,
        D3D11_SDK_VERSION, &sd, &SwapChain, &Device, &FeatureLevel,
        &DeviceContext);
    if (result != S_OK)
    {
        OutputDebugString("[Error] Couldn't create device swapchain");
    }
}

// ================== Platform Provided Services ==================

// Prints a debug message to the windows console using
// OutputDebugString
void PlatformPrintDebug(const char* message)
{
    OutputDebugString(message);
}
