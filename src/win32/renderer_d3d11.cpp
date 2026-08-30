#include <Windows.h>
#include <d3d11.h>

#include "../main.h"
#include "win32_platform.h"

struct Renderer
{
};

// ====================== Internal functions ======================
namespace
{
    // TODO(harsh): return the device, swapchain
    void SetupD3D11AndSwapchain(HWND WindowHandle)
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
} // namespace

// creates a new renderer with the "new" keyword and returns the pointer
// the allocated memory is not tracker you must track and free the renderer
// yourself
Renderer* RendererNew(PlatformWindow* Window)
{
    // TODO(harsh): allocate using a arena allocator
    Renderer* r = new Renderer;

    PlatformPrintDebug("Renderer Init\n");
    SetupD3D11AndSwapchain(Window->Handle);

    return r;
}

void RendererUpdate(Renderer* r, Game* g) {}
