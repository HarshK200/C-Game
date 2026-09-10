#include <assert.h>

// utils
#include "src/game2d/camera2d.h"
#include "src/utils/game_math.h"
#include "src/utils/log.h"
#include "src/utils/globals.h"
#include "src/utils/arena_allocator.h"

#include "src/main.h"
#include "src/game2d/game2d.h"
#include "src/win32/renderer/mesh.h"
#include "src/win32/win32_platform.h"
#include "src/win32/renderer/texture_d3d11.h"
#include "src/win32/renderer/renderer_d3d11.h"


// ====================== Internal functions ======================
namespace
{


    /*
        Sets up DirectX11 by creating the swapchain, Device.
        Writes the resulting pointers to the Renderer passed in.

        Returns S_OK if succeeds else returns the HRESULT i.e. the error code on failure.
    */
    HRESULT SetupD3D11(HWND window_handle, Renderer* r)
    {

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = DEFAULT_WINDOW_RESOLUTION.x;
        sd.BufferDesc.Height = DEFAULT_WINDOW_RESOLUTION.y;
        // NOTE(harsh): sRGB is non-linear color encoding as human eye's are more sensitive to darker tones than
        // brighter tones, when creating textures make sure to specifiy DXGI_FORMAT_R8G8B8A8_UNORM_SRGB in texture
        // description the GPU will handle the sRGB -> Linear conversion
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = window_handle;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = true;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0; // Only support D3D11 features

        UINT CreateDeviceFlags = 0;
#if defined(ISEKAIED_DEBUG)
        // for D3D11 debug output
        CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // create the device and swapchain
        // don't need the returned feature level hence passing NULL
        HRESULT result = D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            CreateDeviceFlags,
            &FeatureLevels,
            1,
            D3D11_SDK_VERSION,
            &sd,
            &r->SwapChain,
            &r->Device, NULL,
            &r->DeviceContext);
        if (FAILED(result))
        {
            return result;
        }
        assert(SUCCEEDED(result) && r->SwapChain && r->Device && r->DeviceContext);

        return S_OK;
    }


    /*
        Creates the Render textures i.e. BackBufferRender texture & InternalRenderTexture and the
        RenderTargetView/ShaderResourceView i.e. InteralRenderTextureRTV & InternalRenderTextureSRV for the
        InternalRenderTexture AND,
        Sets the created Render Textures in the appropriate field on the renderer

        On Sucess returns S_OK, otherwise returns the failure HRESULT code
    */
    HRESULT CreateAndSetRenderTextures(Renderer* r)
    {
        HRESULT result;

        // creating internal render texture
        D3D11_TEXTURE2D_DESC internal_texture_desc = {};
        internal_texture_desc.Width = 640;
        internal_texture_desc.Height = 360;
        internal_texture_desc.MipLevels = 1;
        internal_texture_desc.ArraySize = 1;
        internal_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        internal_texture_desc.SampleDesc.Count = 1;
        internal_texture_desc.Usage = D3D11_USAGE_DEFAULT;
        internal_texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        result = r->Device->CreateTexture2D(&internal_texture_desc, NULL, &r->InternalRenderTexture);
        if (FAILED(result))
        {
            LOG_ERRORF("Creating internal render texture FAILED! with error code: %d", result);
            return result;
        }

        // create render target view and shader resource view (not passing any desc so just default)
        D3D11_RENDER_TARGET_VIEW_DESC internal_rtv_desc = {};
        internal_rtv_desc.Format = internal_texture_desc.Format;
        internal_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        result = r->Device->CreateRenderTargetView(r->InternalRenderTexture, &internal_rtv_desc, &r->InternalRTV);
        if (FAILED(result))
        {
            LOG_ERRORF("Creating internal_texture RenderTargetView FAILED! with error code: %d", result);
            return result;
        }
        D3D11_SHADER_RESOURCE_VIEW_DESC internal_srv_desc = {};
        internal_srv_desc.Format = internal_texture_desc.Format;
        internal_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        internal_srv_desc.Texture2D.MostDetailedMip = 0;
        internal_srv_desc.Texture2D.MipLevels = 1;
        result = r->Device->CreateShaderResourceView(r->InternalRenderTexture, &internal_srv_desc, &r->InternalSRV);
        if (FAILED(result))
        {
            LOG_ERRORF("Creating internal_texture ShaderResourceView FAILED! with error code: %d", result);
            return result;
        }


        // Get the pointer to the back-buffer
        ID3D11Texture2D* back_buffer_texture;
        result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);
        if (FAILED(result))
        {
            LOG_ERRORF("SetupPixelartRenderTargets SwapChain Getting buffer FAILED! with error code: %d", result);
            return result;
        }

        // Create RenderTargetView for the back-buffer
        D3D11_RENDER_TARGET_VIEW_DESC backbuffer_rtv_desc = {};
        // the backbuffer is sRGB so all the internal linear calculations for lighting
        // finally converts to sRGB encoded colors
        backbuffer_rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        backbuffer_rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        result = r->Device->CreateRenderTargetView(
            back_buffer_texture,
            &backbuffer_rtv_desc,
            &r->BackBufferRTV);
        if (FAILED(result))
        {
            LOG_ERRORF("SetupPixelartRenderTargets Render Target View Creation FAILED! with error code: %d", result);
            return result;
        }
        back_buffer_texture->Release();

        return S_OK;
    }

    /*
        Creates the point sampler and sets it on the Renderer PointSampler field
        On Sucess returns S_OK, otherwise returns the failure HRESULT code

        NOTE(harsh): the internal render texture is non sRGB for linear lighting/blending calculations
        and the final back buffer Render Target View is sRGB so all the linear values get encoded to sRGB again.
    */
    HRESULT CreateAndSetPointSampler(Renderer* r)
    {
        D3D11_SAMPLER_DESC sd = {};
        // NOTE(harsh): nearest-neighbour sampling, no blurring
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        HRESULT result = r->Device->CreateSamplerState(&sd, &r->PointSampler);
        if (FAILED(result))
        {
            return result;
        }

        return S_OK;
    }

    HRESULT CreateUniformBuffers(Renderer* r)
    {
        return S_OK;
    }

    void RenderPass_Game(Renderer* r, Game2d* g)
    {
        // ========================= Internal Render texture setup =========================

        // set internal texture as render target
        r->DeviceContext->OMSetRenderTargets(1, &r->InternalRTV, NULL);
        // clear the internal render target with black color
        float background_colour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        r->DeviceContext->ClearRenderTargetView(r->InternalRTV, background_colour);
        // NOTE(harsh): the internal render resolution is set here that determines
        // the aspect ratio of the viewport
        D3D11_VIEWPORT internal_render_viewport = {
            0.0f,
            0.0f,
            INTERNAL_RENDER_RESOLUTION.x,
            INTERNAL_RENDER_RESOLUTION.y,
            0.0f,
            1.0f,
        };
        r->DeviceContext->RSSetViewports(1, &internal_render_viewport);
        // set the topology for draw calls
        r->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


        // ============================ Upload Per Frame Uniforms ============================
        FrameUniforms frame_uniforms = {};
        frame_uniforms.View = Camera2dGetViewMatrix(g->camera);
        frame_uniforms.Projection = Orthograhpic_RH_ZO_Mat4(
            0,
            INTERNAL_RENDER_RESOLUTION.x,
            INTERNAL_RENDER_RESOLUTION.y,
            0,
            g->camera->near_plane,
            g->camera->far_plane);
        ID3D11Buffer* uniform_buffer = nullptr;
        D3D11_BUFFER_DESC uniforms_buffer_desc = {};
        uniforms_buffer_desc.ByteWidth = sizeof(FrameUniforms);
        uniforms_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        // in D3D11 uniforms are CONSTANT BUFFERS
        uniforms_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        uniforms_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        HRESULT result = r->Device->CreateBuffer(
            &uniforms_buffer_desc,
            NULL,
            &uniform_buffer);

        // upload the uniform data


        // ========================= Setup Shaders and Input Layout =========================

        Shader* default_shader = r->Shaders[SHADER_DEFAULT];
        r->DeviceContext->VSSetShader(default_shader->VertexShader, NULL, 0);
        r->DeviceContext->PSSetShader(default_shader->PixelShader, NULL, 0);
        r->DeviceContext->IASetInputLayout(default_shader->InputLayout);

        // draw calls ==================== TEMP QUAD DRAW CALL INLINE ====================
        {
            // bind the quad vertex buffer for drawing
            r->DeviceContext->IASetVertexBuffers(
                0,
                1,
                &r->QuadMesh->VertexBuffer,
                &r->QuadMesh->VertexStride,
                &r->QuadMesh->VertexOffset);

            // bind the quad index buffer for drawing
            r->DeviceContext->IASetIndexBuffer(
                r->QuadMesh->IndexBuffer,
                DXGI_FORMAT_R32_UINT,
                r->QuadMesh->IndexOffset);

            // bind texture shader resource view and the sampler i.e. PointSampler
            r->DeviceContext->PSSetShaderResources(0, 1, &r->Textures[TEXTURE_ENTITY_ATLAS]->SRV);
            r->DeviceContext->PSSetSamplers(0, 1, &r->PointSampler);

            // make the draw call
            // TODO(harsh): upload per entity uniforms
            r->DeviceContext->DrawIndexed(
                r->QuadMesh->IndexCount,
                r->QuadMesh->IndexOffset,
                r->QuadMesh->VertexOffset);

            // unbind the TextureSRV
            ID3D11ShaderResourceView* null_srv = NULL;
            r->DeviceContext->PSSetShaderResources(0, 1, &null_srv);
        }
    }

    void RenderPass_Upscale(Renderer* r)
    {
        // set the backbuffer as render target
        r->DeviceContext->OMSetRenderTargets(1, &r->BackBufferRTV, NULL);

        // clear backbuffer render target with black color
        float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        r->DeviceContext->ClearRenderTargetView(r->BackBufferRTV, black);

        // set the viewport that should be the exact same as the window
        // TODO(harsh): make the viewport width and height set by settings.
        D3D11_VIEWPORT backbuffer_render_viewport = {
            0.0f,
            0.0f,
            DEFAULT_WINDOW_RESOLUTION.x,
            DEFAULT_WINDOW_RESOLUTION.y,
            0.0f,
            1.0f,
        };
        r->DeviceContext->RSSetViewports(1, &backbuffer_render_viewport);

        // set the topology for draw calls
        r->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // set the vertex & pixel shader and input layout
        Shader* upscale_shader = r->Shaders[SHADER_UPSCALE];
        r->DeviceContext->VSSetShader(upscale_shader->VertexShader, NULL, 0);
        r->DeviceContext->PSSetShader(upscale_shader->PixelShader, NULL, 0);
        r->DeviceContext->IASetInputLayout(upscale_shader->InputLayout);


        // ========================= DRAW THE UPSCALED TEXTURE =========================

        // bind upscale_quad_mesh vertex buffer
        r->DeviceContext->IASetVertexBuffers(
            0,
            1,
            &r->UpscaleQuadMesh->VertexBuffer,
            &r->UpscaleQuadMesh->VertexStride,
            &r->UpscaleQuadMesh->VertexOffset);

        // bind upscale_quad_mesh index buffer
        r->DeviceContext->IASetIndexBuffer(
            r->UpscaleQuadMesh->IndexBuffer,
            DXGI_FORMAT_R32_UINT,
            r->UpscaleQuadMesh->IndexOffset);

        // bind internal render texture shader resource view and the point sampler
        r->DeviceContext->PSSetShaderResources(0, 1, &r->InternalSRV);
        r->DeviceContext->PSSetSamplers(0, 1, &r->PointSampler);

        // make the upscale draw call
        r->DeviceContext->DrawIndexed(
            r->UpscaleQuadMesh->IndexCount,
            r->UpscaleQuadMesh->IndexOffset,
            r->UpscaleQuadMesh->VertexOffset);

        // unbind InternalSRV — it must be free before next frame's RenderPass_Game else D3D11 gives a warning
        ID3D11ShaderResourceView* null_srv = NULL;
        r->DeviceContext->PSSetShaderResources(0, 1, &null_srv);
    }

} // namespace


// ================== Renderer Layer Services Definitions ==================

/*
    Creates and initializes a D3D11 renderer (allocates renderer).
    Returns Renderer* if succeeds otherwise returns nullptr.

    NOTE(harsh): allocates using provided permanent_allocator for initialization,
    and Any temporary allocation are done using the provided temp_allocator.
*/
Renderer* RendererCreateAndInit(PlatformWindow* window, AppMemory* memory)
{
    LOG_INFO("Renderer Init");
    Renderer* r = (Renderer*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Renderer));

    HRESULT result = SetupD3D11(window->Handle, r);
    if (FAILED(result))
    {
        LOG_ERRORF("SetupD3D11 FAILED! with error code: %d", result);
        return nullptr;
    }

    result = LoadAllShaders(memory, r->Device, r->Shaders);
    if (result == -1)
    {
        LOG_ERRORF("D3D11 LoadAllShaders FAILED! with error code: %d", result);
        return nullptr;
    }

    // load all texture
    int texture_result = LoadAllTextures(memory, r->Device, r->Textures);
    if (texture_result < 0)
    {
        LOG_ERRORF("D3D11 LoadAllTextures FAILED! with error code: %d", result);
        return nullptr;
    }

    result = CreateAndSetRenderTextures(r);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 CreateAndSetRenderTextures FAILED! with error code: %d", result);
        return nullptr;
    }

    result = CreateAndSetPointSampler(r);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 CreateAndSetPointSampler FAILED! with error code: %d", result);
        return nullptr;
    }

    // create uniform buffer
    result = CreateUniformBuffers(r);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 CreateUniformBuffers FAILED! with error code: %d", result);
        return nullptr;
    }

    // upload mesh vertex/index buffers
    r->UpscaleQuadMesh = CreateUpscaleQuadMesh(r->Device, memory);
    r->TriangleMesh = CreateTriangleMesh(r->Device, memory);
    r->QuadMesh = CreateQuadMesh(r->Device, memory);

    return r;
}

/*
    1. GAME_PASS: game render on InternalRenderTexture's RenderTargetView.
    2. UPSCALE_PASS: takes that InternalRenderTexture and pass that to a upscale shader which samples
                     internal render texture with a PointSampler i.e. NearestNeighbourSampling onto the
                     BackBufferRenderTargetView.
    3. Finally present's the backbuffer by DXGI_SWAP_EFFECT_FLIP_DISCARD, switching the backbuffer with front
*/
void RendererUpdate(Renderer* r, Game2d* g, PlatformWindow* window)
{

    // ======================== GAME RENDER PASS ========================
    RenderPass_Game(r, g);
    RenderPass_Upscale(r);


    // VERY IMPORTANT Finally Swap the back-buffer to show it
    r->SwapChain->Present(1, 0);
}
