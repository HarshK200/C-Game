#include <d3d11.h>
#include <d3dcompiler.h>
#include <iterator>

#include "src/main.h"
#include "src/win32/renderer/renderer_d3d11.h"
#include "src/win32/renderer/shader_d3d11.h"


/*
    Compiles the vertex and pixel shaders from shader_file_path using compile_options.
    Sets the resulting shader in the Renderer's Shaders array.
    if input_element_desc != nullptr then the InputLayout for the shader is also created.
    WARNING: the input_element_count must be > 0 when input_element_desc is passed in

    Returns Shader* on success, otherwise returns nullptr on failure.
*/
Shader* CreateShader(
    Renderer* r,
    ShaderID shader_id,
    const wchar_t* shader_file_path,
    UINT compile_options,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT input_element_count)
{
    ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *error_blob = nullptr;
    // TODO(harsh): use Arena allocator for this shader allocation
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
        // PlatformPrintDebug((char*)error_blob->GetBufferPointer());
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

cleanup:
    if (error_blob)
    {
        // PlatformPrintDebug((char*)error_blob->GetBufferPointer());
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

        return nullptr;
    }

    return shader;
}


/*
    Loads all the vertex & pixel shaders TODO(harsh): implemente shader compilation caching andload from cache.
    If no cache found Compiles the shaders and creates there input layouts.
    Creates a Shader struct containing pointers to the input_layout, vertex & fragment shaders,
    and writes them into the Shaders[] on the renderer

    On Success returns 0, otherwise returns -1 on failure.
*/
int LoadAllShaders(Renderer* r)
{
    UINT compile_options = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(ISEKAIED_DEBUG)
    compile_options |= D3DCOMPILE_DEBUG;
#endif


    // loading default shader
    D3D11_INPUT_ELEMENT_DESC default_input_element_desc[] = {
        {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Shader* default_shader = CreateShader(
        r,
        Shader_Default,
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/default.hlsl",
        compile_options,
        default_input_element_desc,
        std::size(default_input_element_desc));
    if (default_shader == nullptr)
        return -1;
    r->Shaders[Shader_Default] = default_shader;

    // loading pixelart upscale shader
    D3D11_INPUT_ELEMENT_DESC upscale_input_element_desc[] = {
        {"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Shader* upscale_shader = CreateShader(
        r,
        Shader_Upscale,
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/upscale.hlsl",
        compile_options,
        upscale_input_element_desc,
        std::size(upscale_input_element_desc));
    if (upscale_shader == nullptr)
        return -1;
    r->Shaders[Shader_Upscale] = upscale_shader;

    return 0;
}
