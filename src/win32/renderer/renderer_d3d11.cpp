#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <assert.h>
#include <iterator>

#include "src/main.h"
#include "src/win32/win32_platform.h"
#include "src/win32/renderer/renderer_d3d11.h"


// ====================== Internal functions ======================
namespace
{


    /*
        Compiles a single shader file's Vertex and Pixel Shader at shader_file_path with compile_options.
        It Sets teh shader in the Renderer's Shaders array
        On Success returns S_OK otherwise HRESULT
    */
    HRESULT CreateShader(
        Renderer* r,
        ShaderID shader_id,
        const wchar_t* shader_file_path,
        UINT compile_options,
        D3D11_INPUT_ELEMENT_DESC* input_element_desc,
        UINT input_element_count)
    {
        Shader* shader = new Shader{};
        ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *error_blob = nullptr;


        // compile vertex shader
        HRESULT result = D3DCompileFromFile(
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
        {
            if (error_blob)
            {
                // basically casting the buffer pointer to a char* so its
                // treated as a char array i.e. string
                PlatformPrintDebugF("[ERROR] Failed compiling Vertex Shader at path: %s", shader_file_path);
                PlatformPrintDebug((char*)error_blob->GetBufferPointer());
                error_blob->Release();
            }
            // release vs_blob if complie failed
            if (vs_blob)
                vs_blob->Release();
            delete shader;
            return result;
        }
        ID3D11VertexShader* vertex_shader = nullptr;
        result = r->Device->CreateVertexShader(
            vs_blob->GetBufferPointer(),
            vs_blob->GetBufferSize(),
            NULL,
            &vertex_shader);
        if (FAILED(result))
        {
            // release vs_blob if CreateVertexShader failed
            if (vs_blob)
                vs_blob->Release();
            delete shader;
            return result;
        }
        shader->VertexShader = vertex_shader;


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
        {
            if (error_blob)
            {
                // basically casting the buffer pointer to a char* so its
                // treated as a char array i.e. string
                PlatformPrintDebugF("[ERROR] Failed compiling Pixel Shader at path: %s", shader_file_path);
                PlatformPrintDebug((char*)error_blob->GetBufferPointer());
                error_blob->Release();
            }
            // release ps_blob if complie failed
            if (ps_blob)
                ps_blob->Release();
            delete shader;
            return result;
        }
        ID3D11PixelShader* pixel_shader = nullptr;
        result = r->Device->CreatePixelShader(
            ps_blob->GetBufferPointer(),
            ps_blob->GetBufferSize(),
            NULL,
            &pixel_shader);
        if (FAILED(result))
        {
            // release ps_blob if CreatePixelShader failed
            if (ps_blob)
                ps_blob->Release();
            delete shader;
            return result;
        }
        shader->PixelShader = pixel_shader;

        // Input Layout setup for the shader
        ID3D11InputLayout* input_layout = nullptr;
        result = r->Device->CreateInputLayout(
            input_element_desc,
            input_element_count,
            vs_blob->GetBufferPointer(),
            vs_blob->GetBufferSize(),
            &input_layout);
        if (FAILED(result))
        {
            delete shader;
            return result;
        }
        shader->InputLayout = input_layout;

        // set the shader in the Renderer shader array
        r->Shaders[shader_id] = shader;

        vs_blob->Release();
        ps_blob->Release();

        return S_OK;
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
        // clang-format off
        #if defined(ISEKAIED_DEBUG)
        // for D3D11 debug output
        CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif
        // clang-format on

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
        // clang-format off
        #if defined(ISEKAIED_DEBUG)
            compile_options |= D3DCOMPILE_DEBUG;
        #endif
        // clang-format on


        // loading default-shader
        D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
            {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
        HRESULT result = CreateShader(
            r,
            Shader_Default,
            L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/default.hlsl",
            compile_options,
            input_element_desc,
            std::size(input_element_desc));
        if (FAILED(result))
        {
            return result;
        }

        return S_OK;
    }


    /*
        Creates the InternalRenderTexture and its RenderTargetView/ShaderResourceView i.e.
        InteralRenderTextureRTV & InternalRenderTextureSRV
    */
    HRESULT CreateRenderTextures(Renderer* r)
    {
        // TODO(harsh): Create the internal render texture


        // Get the pointer to the back-buffer
        ID3D11Texture2D* back_buffer_texture;
        HRESULT result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] SetupPixelartRenderTargets SwapChain Getting buffer FAILED! with error code: %d",
                                result);
            return result;
        }

        // Create RenderTargetView for the back-buffer
        result = r->Device->CreateRenderTargetView(back_buffer_texture, 0, &r->BackBufferRTV);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] SetupPixelartRenderTargets Render Target View Creation FAILED! with error code: %d",
                                result);
            return result;
        }
        back_buffer_texture->Release();

        return S_OK;
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

        // ============================= UPLOADING VERTEX BUFFER =============================
        r->TriangleMesh = new Mesh{};

        // clang-format off
        float vertex_data[] = {
             0.0f,  0.5f, 0.0f, // top (D3D11 Y+ Up convention, this is temporary i'll use Y+ Down Convention later)
             0.5f, -0.5f, 0.0f, // bottom-right
            -0.5f, -0.5f, 0.0f, // bottom-left
        };
        // clang-format on
        r->TriangleMesh->VertexBuffer = nullptr;
        {
            D3D11_BUFFER_DESC vertex_buf_desc = {};
            vertex_buf_desc.ByteWidth = sizeof(vertex_data);
            vertex_buf_desc.Usage = D3D11_USAGE_IMMUTABLE;
            vertex_buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA sr_data = {0};
            sr_data.pSysMem = vertex_data;

            result = r->Device->CreateBuffer(&vertex_buf_desc, &sr_data, &r->TriangleMesh->VertexBuffer);
            if (FAILED(result))
            {
                PlatformPrintDebugF(
                    "[ERROR] D3D11 Triangle Vertex Buffer creation FAILED! with error code: %d", result);
                return nullptr;
            }
        }
        // ===================================================================================
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
    // bind the render target NOTE(harsh): MUST do this before any draw or clear calls
    r->DeviceContext->OMSetRenderTargets(1, &r->BackBufferRTV, NULL);

    // clear the screen with cornflower blue
    float background_colour[4] = {119.0f / 255.0f, 221.0f / 255.0f, 119.0f / 255.0f, 1.0f};
    r->DeviceContext->ClearRenderTargetView(r->BackBufferRTV, background_colour);

    // set the viewport for drawing
    RECT win_rect;
    GetClientRect(window->Handle, &win_rect);
    D3D11_VIEWPORT viewport = {
        0.0f,
        0.0f,
        640.0f,
        360.0f,
        0.0f,
        1.0f,
    };
    r->DeviceContext->RSSetViewports(1, &viewport);

    // set primitive topology to draw triangles
    r->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // set the shader input layout
    r->DeviceContext->IASetInputLayout(r->Shaders[0]->InputLayout);

    // set the vertex buffer, the vertex shader and the pixel shader to use for the draw call
    // TODO(harsh): move all this data to the Shader struct
    UINT vertex_stride = 3 * sizeof(float);
    UINT vertex_offset = 0;
    UINT vertex_count = 3;
    r->DeviceContext->IASetVertexBuffers(0, 1, &r->TriangleMesh->VertexBuffer, &vertex_stride, &vertex_offset);
    r->DeviceContext->VSSetShader(r->Shaders[0]->VertexShader, NULL, 0);
    r->DeviceContext->PSSetShader(r->Shaders[0]->PixelShader, NULL, 0);

    // =============== FINAL DRAW CALL!!! ===============
    r->DeviceContext->Draw(vertex_count, vertex_offset);

    // VERY IMPORTANT Finally Swap the back-buffer to show it
    r->SwapChain->Present(1, 0);
}
