#include <assert.h>
#include <d3d11.h>
#include <dxgiformat.h>

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.h"

// Platform specific import
#include "src/win32/win32_platform.h"

#include "src/main.h"
#include "src/renderer/d3d11/mesh.h"
#include "src/renderer/d3d11/shader_d3d11.h"
#include "src/renderer/d3d11/texture_d3d11.h"


struct Renderer
{
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;

    ID3D11RenderTargetView* BackBufferRTV;
    ID3D11Texture2D* InternalRenderTexture; // 640x360 i.e. 16:9 aspect ratio
    ID3D11RenderTargetView* InternalRTV;
    ID3D11ShaderResourceView* InternalSRV;

    Shader* Shaders[SHADER_COUNT];
    ID3D11Buffer* UniformBuffers[UNIFORM_BUFFER_COUNT];
    Texture2D* Textures[TEXTURE_COUNT];
    Mesh* Meshes[MESH_COUNT];
    ID3D11SamplerState* PointSampler; // TODO(harsh): maybe create a ID3D11SamplerState* array like the shader arary?
};

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
        internal_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
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

    void RenderPass_Game(Renderer* r, RenderData* render_data)
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


        // ========================= Upload Per Frame Uniforms Buffer =========================

        // upload the Per Frame Uniform Buffer data
        FrameUniforms frame_uniforms = {};
        frame_uniforms.View = ViewMat4(
            render_data->view_matrix_params.position,
            render_data->view_matrix_params.offset,
            render_data->view_matrix_params.zoom);
        frame_uniforms.Projection = Orthograhpic_RH_ZO_Mat4(
            0,
            INTERNAL_RENDER_RESOLUTION.x,
            INTERNAL_RENDER_RESOLUTION.y,
            0,
            render_data->projection_matrix_params.near_plane,
            render_data->projection_matrix_params.far_plane);
        UploadUniformBufferData(
            r->DeviceContext,
            r->UniformBuffers[UNIFORM_PER_FRAME_BUFFER],
            frame_uniforms);
        // bind the Per Frame Uniform Buffer. NOTE(harsh): no unbind required
        r->DeviceContext->VSSetConstantBuffers(0, 1, &r->UniformBuffers[UNIFORM_PER_FRAME_BUFFER]);


        // ========================= Default Shader Draw Pipeline =========================

        ID3D11ShaderResourceView* null_srv = NULL;

        // bind default shader and input layout
        Shader* default_shader = r->Shaders[SHADER_DEFAULT];
        r->DeviceContext->VSSetShader(default_shader->VertexShader, NULL, 0);
        r->DeviceContext->PSSetShader(default_shader->PixelShader, NULL, 0);
        r->DeviceContext->IASetInputLayout(default_shader->InputLayout);

        /*
            NOTE(harsh): looping through all the render commands and rendering them.

            TODO(harsh): render in the correct sort order and add ShaderGroups
        */
        for (int i = 0; i < render_data->commands_count; i++)
        {
            RenderCommand render_command = render_data->render_commands[i];
            Mesh* mesh = r->Meshes[render_command.mesh_id];
            Texture2D* texture = r->Textures[render_command.texture_id];


            // bind Mesh
            r->DeviceContext->IASetVertexBuffers(
                0,
                1,
                &mesh->VertexBuffer,
                &mesh->VertexStride,
                &mesh->VertexOffset);
            r->DeviceContext->IASetIndexBuffer(
                mesh->IndexBuffer,
                mesh->IndexFormat,
                mesh->IndexOffset);

            // bind Texture
            r->DeviceContext->PSSetShaderResources(0, 1, &texture->SRV);

            // bind Sampler (only one single point sampler is used)
            r->DeviceContext->PSSetSamplers(0, 1, &r->PointSampler);

            // Upload & Bind Entity uniforms
            EntityUniforms entity_uniforms = {};
            entity_uniforms.Model = render_command.transform;
            entity_uniforms.UVMinMax = render_command.uv_min_max;
            UploadUniformBufferData(
                r->DeviceContext,
                r->UniformBuffers[UNIFORM_PER_ENTITY_BUFFER],
                entity_uniforms);
            r->DeviceContext->VSSetConstantBuffers(1, 1, &r->UniformBuffers[UNIFORM_PER_ENTITY_BUFFER]);

            // Make the draw call
            LOG_ASSERT(mesh->IndexCount != 0, "Error mesh doesn't support index drawing");
            r->DeviceContext->DrawIndexed(
                mesh->IndexCount,
                mesh->IndexOffset,
                mesh->VertexOffset);

            // Unbind the TextureSRV
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
            &r->Meshes[MESH_UPSCALE_QUAD]->VertexBuffer,
            &r->Meshes[MESH_UPSCALE_QUAD]->VertexStride,
            &r->Meshes[MESH_UPSCALE_QUAD]->VertexOffset);

        // bind upscale_quad_mesh index buffer
        r->DeviceContext->IASetIndexBuffer(
            r->Meshes[MESH_UPSCALE_QUAD]->IndexBuffer,
            DXGI_FORMAT_R32_UINT,
            r->Meshes[MESH_UPSCALE_QUAD]->IndexOffset);

        // bind internal render texture shader resource view and the point sampler
        r->DeviceContext->PSSetShaderResources(0, 1, &r->InternalSRV);
        r->DeviceContext->PSSetSamplers(0, 1, &r->PointSampler);

        // make the upscale draw call
        r->DeviceContext->DrawIndexed(
            r->Meshes[MESH_UPSCALE_QUAD]->IndexCount,
            r->Meshes[MESH_UPSCALE_QUAD]->IndexOffset,
            r->Meshes[MESH_UPSCALE_QUAD]->VertexOffset);

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

    // load all texture
    int texture_result = LoadAllTextures(memory, r->Device, r->Textures);
    if (texture_result < 0)
    {
        LOG_ERRORF("D3D11 LoadAllTextures FAILED! with error code: %d", result);
        return nullptr;
    }

    // upload mesh vertex/index buffers
    result = CreateAllMeshs(memory, r->Device, r->Meshes);
    if (result == -1)
    {
        LOG_ERRORF("D3D11 LoadAllShaders FAILED! with error code: %d", result);
        return nullptr;
    }

    result = LoadAllShaders(memory, r->Device, r->Shaders);
    if (result == -1)
    {
        LOG_ERRORF("D3D11 LoadAllShaders FAILED! with error code: %d", result);
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

    result = CreateAllUniformBuffers(r->Device, r->UniformBuffers);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 CreateUniformBuffers FAILED! with error code: %d", result);
        return nullptr;
    }


    return r;
}

/*
    1. GAME_PASS: game render on InternalRenderTexture's RenderTargetView.
    2. UPSCALE_PASS: takes that InternalRenderTexture and pass that to a upscale shader which samples
                     internal render texture with a PointSampler i.e. NearestNeighbourSampling onto the
                     BackBufferRenderTargetView.
    3. Finally present's the backbuffer by DXGI_SWAP_EFFECT_FLIP_DISCARD, switching the backbuffer with front
*/
void RendererUpdate(PlatformWindow* window, Renderer* r, RenderData* render_data)
{
    // ======================== GAME RENDER PASS ========================
    RenderPass_Game(r, render_data);
    RenderPass_Upscale(r);


    // VERY IMPORTANT Finally Swap the back-buffer to show it
    r->SwapChain->Present(0, 0);
}
