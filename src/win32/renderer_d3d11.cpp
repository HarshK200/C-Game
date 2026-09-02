#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <assert.h>

#include "../main.h"
#include "win32_platform.h"

// TODO(harsh): Temporarly placed here, move these structs to there
// dedicated file, like shader_d3d11.cpp, mesh_d3d11.h and triangle_mesh_d3d11.cpp
// which includes the mesh_d3d11.h file for Mesh struct definition
struct Shader
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
};
// TODO(harsh): make a CreateMesh function which will upload the mesh's vertex data
struct Mesh
{
    ID3D11Buffer* VertexBuffer;
};


struct Renderer
{
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;
    ID3D11RenderTargetView* RenderTargetView;
    Shader* Shaders[2];

    Mesh* TriangleMesh;
};


// ====================== Internal functions ======================
namespace
{
    /*
       Sets up DirectX11 by creating the swapchain, Device and the render target view.
       Writes the resulting pointers to the Renderer passed in.
       Returns S_OK if succeeds else returns the HRESULT i.e. the error code on failure.
    */
    HRESULT SetupD3D11(HWND window_handle, Renderer* r)
    {

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = 640;
        sd.BufferDesc.Height = 360;
        // TODO(harsh): what's SRGB? also the article said the non SRGB one has
        // gamma correction issues?
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = window_handle;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = true;

        // Only support D3D11 features
        D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
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


        // create the frame_buffer to render to
        ID3D11Texture2D* frame_buffer;
        result = r->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] D3D11 SwapChain Getting buffer FAILED! with error code: %d",
                                result);
            return result;
        }

        // create the Render Target using the frame buffer
        result = r->Device->CreateRenderTargetView(frame_buffer, 0, &r->RenderTargetView);
        if (FAILED(result))
        {
            PlatformPrintDebugF("[ERROR] D3D11 Render Target Creation FAILED! with error code: %d",
                                result);
            return result;
        }
        frame_buffer->Release();


        return S_OK;
    }


    /*
       Compiles all the vertex and pixel shaders and pushes them to the renderer vertex &
       pixel shaders map
    */
    HRESULT CompileAndSetupShaders(Renderer* r)
    {
        UINT CompileOptions = D3DCOMPILE_ENABLE_STRICTNESS;
        // clang-format off
        #if defined(ISEKAIED_DEBUG)
            CompileOptions |= D3DCOMPILE_DEBUG;
        #endif
        // clang-format on

        // NOTE(harsh): Default vertex and pixel shader
        {
            Shader* default_shader = new Shader{};
            ID3DBlob *vs_blob = NULL, *ps_blob = NULL, *error_blob = NULL;

            // Compile and Create default-vertex-shader
            HRESULT result = D3DCompileFromFile(
                L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/shaders/default.hlsl",
                NULL,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                "vs_main",
                "vs_5_0",
                CompileOptions,
                NULL,
                &vs_blob,
                &error_blob);
            if (FAILED(result))
            {
                if (error_blob)
                {
                    // basically casting the buffer pointer to a char* so its
                    // treated as a char array i.e. string
                    PlatformPrintDebug((char*)error_blob->GetBufferPointer());
                    error_blob->Release();
                }
                // release vs_blob if complie failed
                if (vs_blob)
                {
                    vs_blob->Release();
                }
                return result;
            }
            ID3D11VertexShader* vertex_shader = NULL;
            result = r->Device->CreateVertexShader(
                vs_blob->GetBufferPointer(),
                vs_blob->GetBufferSize(),
                NULL,
                &vertex_shader);
            if (FAILED(result))
            {
                return result;
            }
            default_shader->VertexShader = vertex_shader;

            // NOTE(harsh): Compile and create defualt-pixel-shader
            result = D3DCompileFromFile(
                L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/shaders/default.hlsl",
                NULL,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                "ps_main",
                "ps_5_0",
                CompileOptions,
                NULL,
                &ps_blob,
                &error_blob);
            if (FAILED(result))
            {
                if (error_blob)
                {
                    // basically casting the buffer pointer to a char* so its
                    // treated as a char array i.e. string
                    PlatformPrintDebug((char*)error_blob->GetBufferPointer());
                    error_blob->Release();
                }
                // release vs_blob if complie failed
                if (ps_blob)
                {
                    ps_blob->Release();
                }
                return result;
            }
            ID3D11PixelShader* pixel_shader = NULL;
            result = r->Device->CreatePixelShader(
                ps_blob->GetBufferPointer(),
                ps_blob->GetBufferSize(),
                NULL,
                &pixel_shader);
            if (FAILED(result))
            {
                return result;
            }
            default_shader->PixelShader = pixel_shader;


            // NOTE(harsh): Input Layout setup for the shader
            ID3D11InputLayout* input_layout = NULL;
            D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
                {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
            result = r->Device->CreateInputLayout(
                input_element_desc,
                ARRAYSIZE(input_element_desc),
                vs_blob->GetBufferPointer(),
                vs_blob->GetBufferSize(),
                &input_layout);
            if (FAILED(result))
            {
                return result;
            }
            default_shader->InputLayout = input_layout;
            r->Shaders[0] = default_shader;
        }

        return S_OK;
    }

} // namespace


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
            "[ERROR] D3D11 and Swapchain Setup/Creation FAILED! with error code: %d", result);
        return nullptr;
    }

    result = CompileAndSetupShaders(r);
    if (FAILED(result))
    {
        PlatformPrintDebugF(
            "[ERROR] D3D11 Shader Compilation/Setup FAILED! with error code: %d", result);
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
        r->TriangleMesh->VertexBuffer = NULL;
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
    }


    return r;
}

void RendererUpdate(Renderer* r, Game* g, PlatformWindow* window)
{
    // ============================= FINALLY DRAWING =============================

    // clear the screen with pastel green
    // float background_color[4] = {218.0f, 242.0f, 212.0f, 1.0f};
    // r->DeviceContext->ClearRenderTargetView(r->RenderTargetView, background_color);
    float background_colour[4] = {0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f};
    r->DeviceContext->ClearRenderTargetView(r->RenderTargetView, background_colour);

    // set the valid drawing area
    RECT win_rect;
    GetClientRect(window->Handle, &win_rect);
    D3D11_VIEWPORT viewport = {
        0.0f,
        0.0f,
        (FLOAT)(win_rect.right - win_rect.left),
        (FLOAT)(win_rect.bottom - win_rect.top),
        0.0f,
        1.0f,
    };

    r->DeviceContext->RSSetViewports(1, &viewport);
    r->DeviceContext->OMSetRenderTargets(1, &r->RenderTargetView, NULL);
    r->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    r->DeviceContext->IASetInputLayout(r->Shaders[0]->InputLayout);

    // TODO(harsh): move all this data to the Shader struct
    UINT vertex_stride = 3 * sizeof(float);
    UINT vertex_offset = 0;
    UINT vertex_count = 3;
    r->DeviceContext->IASetVertexBuffers(0, 1, &r->TriangleMesh->VertexBuffer, &vertex_stride, &vertex_offset);
    r->DeviceContext->VSSetShader(r->Shaders[0]->VertexShader, NULL, 0);
    r->DeviceContext->PSSetShader(r->Shaders[0]->PixelShader, NULL, 0);

    // FINAL DRAW LINE!!!
    r->DeviceContext->Draw(vertex_count, vertex_offset);

    // also we kinda need to swap the buffer as well now so it acutally shows up on the screen/viewport
    r->SwapChain->Present(1, 0);
}
