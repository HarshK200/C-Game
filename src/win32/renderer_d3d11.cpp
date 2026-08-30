#include <Windows.h>
#include <d3d11.h>
#include <assert.h>

#include "../main.h"
#include "win32_platform.h"


struct Renderer
{
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;
    ID3D11RenderTargetView* RenderTargetView;
};


// ====================== Internal functions ======================
namespace
{
    /*
      TODO(harsh): return the device, swapchain
      Setups up DirectX11 by creating the swapchain and Device, also Create's
      the render target and updates the Renderer passed in
    */
    int SetupD3D11AndSwapchain(HWND WindowHandle, Renderer* r)
    {

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = 640;
        sd.BufferDesc.Height = 480;
        // TODO(harsh): what's SRGB? also the article said the non SRGB one has
        // gamma correction issues?
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = WindowHandle;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = true;

        // NOTE(harsh): Only support D3D11 features
        D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
        // TODO(harsh): check if _Debug is defined and only then set DEBUG flag
        // for create device
        UINT flags = D3D11_CREATE_DEVICE_DEBUG;

        // NOTE(harsh): create the device and swapchain
        // don't need the returned feature level hence passing NULL
        HRESULT result = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, &FeatureLevels, 1,
            D3D11_SDK_VERSION, &sd, &r->SwapChain, &r->Device, NULL,
            &r->DeviceContext);
        if (result != S_OK)
        {
            return -1;
        }
        assert(result == S_OK && r->SwapChain && r->Device && r->DeviceContext);

        return 0;
    }
} // namespace


/*
  creates and initializes a new renderer with the allocates with "new" keyword
  and returns the pointer to it NOTE(harsh): the allocated memory is not
  tracker you must track and free the renderer yourself or use an arena
  allocater, i gotta imlement that allocater first > o <

  Returns Renderer* if succeeds otherwise returns nullptr
*/
Renderer* RendererCreateAndInit(PlatformWindow* Window)
{
    PlatformPrintDebug("Renderer Init\n");

    // TODO(harsh): allocate using a arena allocator
    Renderer* r = new Renderer{};
    if (SetupD3D11AndSwapchain(Window->Handle, r) != 0)
    {
        return nullptr;
    }


    // TODO(harsh): create render target
    ID3D11Texture2D* FrameBuffer;

    return r;
}

void RendererUpdate(Renderer* r, Game* g) {}
