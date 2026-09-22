#include "src/pch.h"

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

// layers glue
#include "src/renderer/render_data.h"

// platform specific import
#include "src/platform/win32/win32_platform.h"

// internal renderer sub_modules implementations
#include "src/renderer/d3d11/mesh_d3d11.cpp"
#include "src/renderer/d3d11/shader_d3d11.cpp"
#include "src/renderer/d3d11/texture_d3d11.cpp"


// =================================================================================
//                      RENDERER LAYER STRUCT DEFINITIONS
// =================================================================================
struct Renderer
{
    Vec2 WinClientRectSize; // backbuffer is also the same size as window since its reset to WinClientRectSize on resize
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;

    ID3D11RenderTargetView* BackBufferRTV;
    ID3D11Texture2D* InternalRenderTexture; // INTERNAL_RENDER_RESOLUTION.x & INTERNAL_RENDER_RESOLUTION.y
    ID3D11RenderTargetView* InternalRTV;
    ID3D11ShaderResourceView* InternalSRV;

    Shader* Shaders[SHADER_COUNT];
    ID3D11Buffer* UniformBuffers[UNIFORM_BUFFER_COUNT];

    Mesh* Meshes[MESH_COUNT];
    Texture2D* Textures[TEXTURE_COUNT];
    ID3D11SamplerState* PointSampler; // TODO(harsh): maybe create a ID3D11SamplerState* array like the shader arary?
};


// =================================================================================
//                              INTERNAL FUNCTIONS
// =================================================================================


/*
    Sets up DirectX11 by creating the swapchain, Device.
    Writes the resulting pointers to the Renderer passed in.

    Returns S_OK if succeeds else returns the HRESULT i.e. the error code on failure.
*/
HRESULT SetupD3D11(HWND window_handle, Renderer* r)
{

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    r->WinClientRectSize = {(int)DEFAULT_WINDOW_RESOLUTION.x, (int)DEFAULT_WINDOW_RESOLUTION.y};
    sd.BufferDesc.Width = (int)DEFAULT_WINDOW_RESOLUTION.x;
    sd.BufferDesc.Height = (int)DEFAULT_WINDOW_RESOLUTION.y;
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
    Resizes the backbuffer and recreates its RenderTargetView
*/
HRESULT HandleWindowResized(Renderer* r, PlatformWindow* window)
{
    // NOTE(harsh): this is DIFFERENT from window rect, this one doesn't include borders and title.
    // Only the internal rect used for rendering
    RECT client_rect;
    GetClientRect(window->Handle, &client_rect);
    Vec2 client_rect_size = {
        (float)(client_rect.right - client_rect.left), // width
        (float)(client_rect.bottom - client_rect.top), // height
    };
    r->WinClientRectSize = client_rect_size;

    // release previous backbuffer render target view, so backbuffer can be resized
    r->BackBufferRTV->Release();
    r->BackBufferRTV = nullptr;

    // resize the backbuffer and recreate its render target view
    r->SwapChain->ResizeBuffers(
        2,
        r->WinClientRectSize.x,
        r->WinClientRectSize.y,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        NULL);
    // Get the pointer to the back-buffer
    ID3D11Texture2D* back_buffer_texture;
    HRESULT result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);
    if (FAILED(result))
    {
        LOG_ERRORF("HandleResizeWindow Getting SwapChain back-buffer FAILED! with error code: %d", result);
        return result;
    }
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
        LOG_ERRORF("HandleResizeWindow back-buffer Render Target View Creation FAILED! with error code: %d", result);
        return result;
    }
    back_buffer_texture->Release();

    return S_OK;
}

/*
    Calculates the letterbox viewport buffer for window and returns that
*/
D3D11_VIEWPORT GetBackbufferViewportLetterbox(Renderer* r)
{
    // LOG_INFOF(
    //     "Client size: %.0f x %.0f, Internal: %.0f x %.0f",
    //     r->WinClientRectSize.width,
    //     r->WinClientRectSize.height,
    //     INTERNAL_RENDER_RESOLUTION.width,
    //     INTERNAL_RENDER_RESOLUTION.height);

    int scale = min(
        (int)r->WinClientRectSize.width / (int)INTERNAL_RENDER_RESOLUTION.width,
        (int)r->WinClientRectSize.height / (int)INTERNAL_RENDER_RESOLUTION.height);

    // Window is smaller than the internal render resolution
    // no upscale required, well just loose part of the rendered texture from screen
    if (scale < 1)
        scale = 1;


    D3D11_VIEWPORT viewport = {};

    viewport.Width = (INTERNAL_RENDER_RESOLUTION.width * scale);
    viewport.Height = (INTERNAL_RENDER_RESOLUTION.height * scale);
    viewport.TopLeftX = ((int)r->WinClientRectSize.width - viewport.Width) / 2;
    viewport.TopLeftY = ((int)r->WinClientRectSize.height - viewport.Height) / 2;

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    return viewport;
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

    // =============== Creating internal render texture ===============
    D3D11_TEXTURE2D_DESC internal_texture_desc = {};
    internal_texture_desc.Width = INTERNAL_RENDER_RESOLUTION.x;
    internal_texture_desc.Height = INTERNAL_RENDER_RESOLUTION.y;
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


    // =============== Create RenderTargetView for the back-buffer ===============
    // Get the pointer to the back-buffer
    ID3D11Texture2D* back_buffer_texture;
    result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);
    if (FAILED(result))
    {
        LOG_ERRORF("SetupPixelartRenderTargets SwapChain Getting buffer FAILED! with error code: %d", result);
        return result;
    }
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

void RenderPass_Game(AppMemory* memory, Renderer* r, RenderData* render_data)
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
        render_data->view_matrix_params.Position,
        render_data->view_matrix_params.Offset,
        render_data->view_matrix_params.Zoom);
    frame_uniforms.Projection = Orthograhpic_RH_ZO_Mat4(
        0,
        INTERNAL_RENDER_RESOLUTION.x,
        INTERNAL_RENDER_RESOLUTION.y,
        0,
        render_data->projection_matrix_params.NearPlane,
        render_data->projection_matrix_params.FarPlane);
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
    // bind Sampler (only one single point sampler is used)
    r->DeviceContext->PSSetSamplers(0, 1, &r->PointSampler);

    // TODO(harsh): sort render commands based on there LayerId


    for (int i = 0; i < render_data->commands_count; i++)
    {
        RenderCommand render_command = render_data->RenderCommands[i];
        Mesh* mesh = r->Meshes[render_command.MeshId];
        Texture2D* texture = r->Textures[render_command.TextureId];

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


        // ===================== Instanced Drawing =====================
        EntityData* entity_data;
        int unsigned instances_to_draw = 0;

        // perpare instance data for only 1 instance
        if (render_command.Instanced == false)
        {
            LOG_ASSERT(
                (render_command.Instanced == false && render_command.NoOfInstances == 0),
                "no_of_instances MUST be 0 when render_command.instanced is false")
            entity_data = ArenaAlloc<EntityData>(&memory->TempAllocator, sizeof(EntityData) * 1);
            instances_to_draw = 1;
            entity_data[0].Model = render_command.Transforms[0];
            entity_data[0].UVMinMax = render_command.UvMinMax[0];
        }

        // prepare instance data for multiple instances
        else
        {
            LOG_ASSERT(
                (render_command.Instanced == true && render_command.NoOfInstances > 1),
                "no_of_instances MUST be above 1 when render_command.instanced is true")
            LOG_ASSERT(render_command.NoOfInstances <= MAX_INSTANCE_BUFFER_SIZE, "no_of_instances EXCEDED MAX_INSTANCE_BUFFER_SIZE")
            entity_data = ArenaAlloc<EntityData>(&memory->TempAllocator, sizeof(EntityData) * render_command.NoOfInstances);
            instances_to_draw = render_command.NoOfInstances;
            for (int i = 0; i < render_command.NoOfInstances; i++)
            {
                entity_data[i].Model = render_command.Transforms[i];
                entity_data[i].UVMinMax = render_command.UvMinMax[i];
            }
        }

        // upload instance vertex buffer and bind instance vertex buffer
        UploadInstanceBufferData(
            r->DeviceContext,
            default_shader->InstanceBuffer,
            entity_data,
            instances_to_draw);
        r->DeviceContext->IASetVertexBuffers(
            1,
            1,
            &default_shader->InstanceBuffer,
            &default_shader->InstanceBufferStride,
            &default_shader->InstanceBufferOffset);


        // Make the draw call
        LOG_ASSERT(mesh->IndexCount != 0, "Error mesh doesn't support index drawing");
        r->DeviceContext->DrawIndexedInstanced(
            mesh->IndexCount,
            instances_to_draw,
            mesh->IndexOffset,
            mesh->VertexOffset,
            0);

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
    D3D11_VIEWPORT backbuffer_render_viewport = GetBackbufferViewportLetterbox(r);
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


// =================================================================================
//                      RENDERER LAYER FUNCTION DEFINITIONS
// =================================================================================
/*
    Creates and initializes a D3D11 renderer (allocates renderer).
    Returns Renderer* if succeeds otherwise returns nullptr.

    NOTE(harsh): allocates using provided permanent_allocator for initialization,
    and Any temporary allocation are done using the provided temp_allocator.
*/
Renderer* RendererCreateAndInit(AppMemory* memory, PlatformWindow* window)
{
    LOG_INFO("Renderer Init");
    Renderer* r = ArenaAlloc<Renderer>(&memory->PermanentAllocator, sizeof(Renderer));

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
void RenderFrame(AppMemory* memory, PlatformWindow* window, Renderer* r, RenderData* render_data, bool window_resized)
{
    if (window_resized)
    {
        HRESULT result = HandleWindowResized(r, window);
        LOG_ASSERT(SUCCEEDED(result), "Failed to resize window with error code: %d", result);
    }

    RenderPass_Game(memory, r, render_data);
    RenderPass_Upscale(r);

    // VERY IMPORTANT Finally Swap the back-buffer to show it
    HRESULT result = r->SwapChain->Present(0, 0); // DXGI_PRESENT_DO_NOT_WAIT flags makes the FPS go Brrrrrrrrr
    LOG_ASSERT(SUCCEEDED(result), "Failed to present a frame with error code: %d", result);
}
