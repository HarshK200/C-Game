#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <assert.h>
#include <dxgiformat.h>
#include <iterator>

#include "src/main.h"
#include "src/game.h"
#include "src/utils/constants.h"
#include "src/win32/win32_platform.h"
#include "src/win32/renderer/renderer_d3d11.h"


// ====================== Internal functions ======================
namespace
{


    /*
        Compiles the vertex and pixel shaders from shader_file_path using compile_options.
        Sets the resulting shader in the Renderer's Shaders array.
        if input_element_desc != nullptr then the InputLayout for the shader is also created.
        WARNING: the input_element_count must be > 0 when input_element_desc is passed in

        Returns S_OK on success, otherwise the failing HRESULT.
    */
    HRESULT CreateShader(
        Renderer* r,
        ShaderID shader_id,
        const wchar_t* shader_file_path,
        UINT compile_options,
        D3D11_INPUT_ELEMENT_DESC* input_element_desc,
        UINT input_element_count)
    {
        ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *error_blob = nullptr;
        Shader* shader = new Shader{};

        HRESULT result;

        // compile vertex shader
        result = D3DCompileFromFile(
            shader_file_path,
            NULL,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "vs_main",
            "vs_5_0",
            compile_options,
            NULL,
            &vs_blob,
            &error_blob);
        if (FAILED(result))
            goto cleanup;
        result = r->Device->CreateVertexShader(
            vs_blob->GetBufferPointer(),
            vs_blob->GetBufferSize(),
            NULL,
            &shader->VertexShader);
        if (FAILED(result))
            goto cleanup;


        // reset the error blob after last call
        if (error_blob)
        {
            PlatformPrintDebug((char*)error_blob->GetBufferPointer());
            error_blob->Release();
            error_blob = nullptr;
        }


        // compile pixel shader
        result = D3DCompileFromFile(
            shader_file_path,
            NULL,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "ps_main",
            "ps_5_0",
            compile_options,
            NULL,
            &ps_blob,
            &error_blob);
        if (FAILED(result))
            goto cleanup;
        result = r->Device->CreatePixelShader(
            ps_blob->GetBufferPointer(),
            ps_blob->GetBufferSize(),
            NULL,
            &shader->PixelShader);
        if (FAILED(result))
            goto cleanup;

        // Input Layout setup for the shader
        if (input_element_desc)
        {
            result = r->Device->CreateInputLayout(
                input_element_desc,
                input_element_count,
                vs_blob->GetBufferPointer(),
                vs_blob->GetBufferSize(),
                &shader->InputLayout);
            if (FAILED(result))
                goto cleanup;
        }

        // set the shader in the Renderer shader array
        // TODO(harsh): use Arena allocator for this shader allocation
        r->Shaders[shader_id] = shader;

    cleanup:
        if (error_blob)
        {
            PlatformPrintDebug((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (vs_blob)
            vs_blob->Release();
        if (ps_blob)
            ps_blob->Release();
        if (FAILED(result))
        {
            if (shader->VertexShader)
                shader->VertexShader->Release();
            if (shader->PixelShader)
                shader->PixelShader->Release();
            if (shader->InputLayout)
                shader->InputLayout->Release();
            delete shader;
        }

        return result;
    }


    /*
        Sets up DirectX11 by creating the swapchain, Device.
        Writes the resulting pointers to the Renderer passed in.

        Returns S_OK if succeeds else returns the HRESULT i.e. the error code on failure.
    */
    HRESULT SetupD3D11(HWND window_handle, Renderer* r)
    {

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        // 16:9 aspect ratio
        sd.BufferDesc.Width = 640;
        sd.BufferDesc.Height = 360;
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
        Loads all the vertex & pixel shaders TODO(harsh): implemente shader compilation caching andload from cache.
        If no cache found Compiles the shaders and creates there input layouts.
        Creates a Shader struct containing pointers to the input_layout, vertex & fragment shaders,
        and writes them into the Shaders[] on the renderer
    */
    HRESULT LoadAllShaders(Renderer* r)
    {
        UINT compile_options = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(ISEKAIED_DEBUG)
        compile_options |= D3DCOMPILE_DEBUG;
#endif


        // loading default shader
        D3D11_INPUT_ELEMENT_DESC default_input_element_desc[] = {
            {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
        HRESULT result = CreateShader(
            r,
            Shader_Default,
            L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/default.hlsl",
            compile_options,
            default_input_element_desc,
            std::size(default_input_element_desc));
        if (FAILED(result))
            return result;

        // loading pixelart upscale shader
        result = CreateShader(
            r,
            Shader_Upscale,
            L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/upscale.hlsl",
            compile_options,
            nullptr,
            NULL);
        if (FAILED(result))
            return result;

        return S_OK;
    }


    /*
        Creates and setups the Render textures i.e. BackBufferRender texture & InternalRenderTexture and the
        RenderTargetView/ShaderResourceView i.e. InteralRenderTextureRTV & InternalRenderTextureSRV for the
        InternalRenderTexture
    */
    HRESULT CreateRenderTextures(Renderer* r)
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
        // TODO(harsh): do i really need default usage here? since i'll use this texture to
        // paint on it?? LOOK AT UP
        internal_texture_desc.Usage = D3D11_USAGE_DEFAULT;
        internal_texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        result = r->Device->CreateTexture2D(&internal_texture_desc, NULL, &r->InternalRenderTexture);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] Creating internal render texture FAILED! with error code: %d", result);
            return result;
        }

        // create render target view and shader resource view (not passing any desc so just default)
        result = r->Device->CreateRenderTargetView(r->InternalRenderTexture, NULL, &r->InternalRTV);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] Creating internal_texture RenderTargetView FAILED! with error code: %d", result);
            return result;
        }
        result = r->Device->CreateShaderResourceView(r->InternalRenderTexture, NULL, &r->InternalSRV);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] Creating internal_texture ShaderResourceView FAILED! with error code: %d", result);
            return result;
        }


        // Get the pointer to the back-buffer
        ID3D11Texture2D* back_buffer_texture;
        result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] SetupPixelartRenderTargets SwapChain Getting buffer FAILED! with error code: %d", result);
            return result;
        }

        // Create RenderTargetView for the back-buffer
        result = r->Device->CreateRenderTargetView(back_buffer_texture, NULL, &r->BackBufferRTV);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] SetupPixelartRenderTargets Render Target View Creation FAILED! with error code: %d", result);
            return result;
        }
        back_buffer_texture->Release();

        return S_OK;
    }


    void RenderPass_Game(Renderer* r, Game* g)
    {
        // set internal texture as render target
        r->DeviceContext->OMSetRenderTargets(1, &r->InternalRTV, NULL);

        // clear the internal render target with pastel green
        float background_colour[4] = {119.0f / 255.0f, 221.0f / 255.0f, 119.0f / 255.0f, 1.0f};
        r->DeviceContext->ClearRenderTargetView(r->InternalRTV, background_colour);

        // set the internal render viewport
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

        // set the input layout
        Shader* default_shader = r->Shaders[Shader_Default];

        // set vertex and pixel shader and input layout to be used
        r->DeviceContext->VSSetShader(default_shader->VertexShader, NULL, 0);
        r->DeviceContext->PSSetShader(default_shader->PixelShader, NULL, 0);
        r->DeviceContext->IASetInputLayout(default_shader->InputLayout);


        // ====================== DRAW CALLS ======================

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
            0);

        // make the draw call
        r->DeviceContext->DrawIndexed(
            r->QuadMesh->IndexCount,
            r->QuadMesh->IndexOffset,
            r->QuadMesh->VertexOffset);
    }

    void RenderPass_Upscale(Renderer* r)
    {
    }

} // namespace


// ================== Renderer Layer Services Definitions ==================

/*
    Creates and initializes a D3D11 renderer (allocates renderer).
    Returns Renderer* if succeeds otherwise returns nullptr.

    NOTE(harsh): the allocated memory is not tracked you must track and free the renderer
    yourself or use an arena allocater, TODO(harsh): i gotta implement that allocater > o <
*/
Renderer* RendererCreateAndInit(PlatformWindow* window)
{
    PlatformPrintDebug("Renderer Init");
    // TODO(harsh): allocate using a arena allocator
    Renderer* r = new Renderer{};

    HRESULT result = SetupD3D11(window->Handle, r);
    if (FAILED(result))
    {
        PlatformPrintDebugF(
            "[ERROR] SetupD3D11 FAILED! with error code: %d", result);
        return nullptr;
    }

    result = LoadAllShaders(r);
    if (FAILED(result))
    {
        PlatformPrintDebugF(
            "[ERROR] D3D11 LoadAllShaders FAILED! with error code: %d", result);
        return nullptr;
    }

    result = CreateRenderTextures(r);
    if (FAILED(result))
    {
        PlatformPrintDebugF(
            "[ERROR] D3D11 CreateRenderTextures FAILED! with error code: %d", result);
        return nullptr;
    }


    // NOTE(harsh): TEMPORARY INLINE UPOLOAD OF TRIANGLE MESH DATA
    // TODO(harsh): create a seprate mesh/triangle_d3d11.cpp file and move this into
    // CreateTriangleMesh function or something
    {

        // ============================= UPLOADING TRIANGLE MESH VERTEX BUFFER =============================

        // TODO(harsh): use arena allocator
        r->TriangleMesh = new Mesh{};

        // vertex buffer data
        // clang-format off
        float triangle_vertex_buffer_data[] = {
             0.0f,  0.5f, 0.0f, // top (D3D11 Y+ Up convention, this is temporary i'll use Y+ Down Convention later)
             0.5f, -0.5f, 0.0f, // bottom-right
            -0.5f, -0.5f, 0.0f, // bottom-left
        };
        // clang-format on
        r->TriangleMesh->VertexBuffer = nullptr;

        // set vertex buffer info
        r->TriangleMesh->VertexStride = sizeof(float) * 3;
        r->TriangleMesh->VertexCount = 3;
        r->TriangleMesh->VertexOffset = 0;

        // upload vertex buffer
        D3D11_BUFFER_DESC triangle_vertex_buff_desc = {};
        triangle_vertex_buff_desc.ByteWidth = sizeof(triangle_vertex_buffer_data);
        triangle_vertex_buff_desc.Usage = D3D11_USAGE_IMMUTABLE;
        triangle_vertex_buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA triangle_vertex_buffer_sr_data = {};
        triangle_vertex_buffer_sr_data.pSysMem = triangle_vertex_buffer_data;
        result = r->Device->CreateBuffer(&triangle_vertex_buff_desc, &triangle_vertex_buffer_sr_data, &r->TriangleMesh->VertexBuffer);
        if (FAILED(result))
        {
            PlatformPrintDebugF(
                "[ERROR] D3D11 Triangle Vertex Buffer creation FAILED! with error code: %d", result);
            return nullptr;
        }


        // ============================= UPLOADING QUAD MESH VERTEX BUFFER =============================

        // TODO(harsh): use arena allocator
        r->QuadMesh = new Mesh{};

        // vertex buffer data
        // clang-format off
        float quad_vertex_buffer_data[] = {
             0.5f,  0.5f, 0.0f, // top-right
             0.5f, -0.5f, 0.0f, // bottom-right
            -0.5f,  0.5f, 0.0f, // top-left
            -0.5f, -0.5f, 0.0f, // bottom-left
        };

        // index buffer data
        // NOTE(harsh): the indices must be in clockwise order for each triangle otherwise it won't be drawn
        // because of back culling.
        int unsigned quad_index_buffer_data[] = {
            2, 0, 1, // top half triangle
            2, 1, 3, // bottom half triangle
        };
        // clang-format on
        r->QuadMesh->VertexBuffer = nullptr;

        // set vertex buffer info
        r->QuadMesh->VertexStride = sizeof(float) * 3;
        r->QuadMesh->VertexCount = 4;
        r->QuadMesh->VertexOffset = 0;

        // upload vertex buffer
        D3D11_BUFFER_DESC quad_vertex_buffer_desc = {};
        quad_vertex_buffer_desc.ByteWidth = sizeof(quad_vertex_buffer_data);
        quad_vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        quad_vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA quad_vertex_buffer_sr_data = {};
        quad_vertex_buffer_sr_data.pSysMem = quad_vertex_buffer_data;
        result = r->Device->CreateBuffer(&quad_vertex_buffer_desc, &quad_vertex_buffer_sr_data, &r->QuadMesh->VertexBuffer);
        if (FAILED(result))
        {
            PlatformPrintDebugF(
                "[ERROR] D3D11 Quad Vertex Buffer creation FAILED! with error code: %d", result);
            return nullptr;
        }

        // set index buffer info
        r->QuadMesh->IndexCount = std::size(quad_index_buffer_data);
        r->QuadMesh->IndexOffset = 0;

        // upload index buffer
        D3D11_BUFFER_DESC quad_index_buffer_desc = {};
        quad_index_buffer_desc.ByteWidth = sizeof(quad_index_buffer_data);
        quad_index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        quad_index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA quad_index_buffer_sr_data = {};
        quad_index_buffer_sr_data.pSysMem = quad_index_buffer_data;
        result = r->Device->CreateBuffer(&quad_index_buffer_desc, &quad_index_buffer_sr_data, &r->QuadMesh->IndexBuffer);
        if (FAILED(result))
        {
            PlatformPrintDebugF(
                "[ERROR] D3D11 Quad Index Buffer creation FAILED! with error code: %d", result);
            return nullptr;
        }
    }


    return r;
}

// TODO(harsh): Setup the 2 pass rendering,
// 1st PASS: game render on InternalRenderTexture's RenderTargetView.
// 2nd PASS: take that InternalRenderTexture and pass that to a blit shader which will sample that
// texture with a PointSampler i.e. NearestNeighbourSampling onto the BackBufferRenderTargetView
// then finally we present the backbuffer
void RendererUpdate(Renderer* r, Game* g, PlatformWindow* window)
{

    // ======================== GAME RENDER PASS ========================
    RenderPass_Game(r, g);


    // VERY IMPORTANT Finally Swap the back-buffer to show it
    r->SwapChain->Present(1, 0);
}
