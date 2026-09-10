#include <d3d11.h>
#include <d3dcompiler.h>
#include <iterator>

#include "src/utils/globals.h"
#include "src/utils/log.h"

#include "src/win32/renderer/shader_d3d11.h"


/*
    Compiles the vertex and pixel shaders from shader_file_path using compile_options.
    Sets the resulting shader in the Renderer's Shaders array.
    if input_element_desc != nullptr then the InputLayout for the shader is also created.
    WARNING: the input_element_count must be > 0 when input_element_desc is passed in

    Returns Shader* on success, otherwise returns nullptr on failure.
*/
Shader* CreateShader(
    ID3D11Device* device,
    AppMemory* memory,
    ShaderID shader_id,
    const wchar_t* shader_file_path,
    UINT compile_options,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT input_element_count)
{
    ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *error_blob = nullptr;
    Shader* shader = (Shader*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Shader));

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
    result = device->CreateVertexShader(
        vs_blob->GetBufferPointer(),
        vs_blob->GetBufferSize(),
        NULL,
        &shader->VertexShader);
    if (FAILED(result))
        goto cleanup;


    // reset the error blob after last call
    if (error_blob)
    {
        LOG_ASSERT(false, (char*)error_blob->GetBufferPointer());
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
    result = device->CreatePixelShader(
        ps_blob->GetBufferPointer(),
        ps_blob->GetBufferSize(),
        NULL,
        &shader->PixelShader);
    if (FAILED(result))
        goto cleanup;

    // Input Layout setup for the shader
    if (input_element_desc)
    {
        result = device->CreateInputLayout(
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
        LOG_ASSERT(false, (char*)error_blob->GetBufferPointer());
        error_blob->Release();
        error_blob = nullptr;
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
    TODO(harsh): implement shader compilation caching,
    also load complied shaders from cache for faster build times.

    Loads all the vertex & pixel shaders.
    If no cache found Compiles the shaders and creates there input layouts.
    Creates a Shader struct containing pointers to the input_layout, vertex & fragment shaders.
    Shader* for the created shaders are stored on the shader_array_buffer passed in

    On Success returns 0, otherwise returns -1 on failure.
*/
int LoadAllShaders(
    AppMemory* memory,
    ID3D11Device* device,
    Shader* (&shader_array_buffer)[SHADER_COUNT])
{
    UINT compile_options = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(ISEKAIED_DEBUG)
    compile_options |= D3DCOMPILE_DEBUG;
#endif

    // loading default shader
    D3D11_INPUT_ELEMENT_DESC default_input_element_desc[] = {
        {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Shader* default_shader = CreateShader(
        device,
        memory,
        SHADER_DEFAULT,
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/default.hlsl",
        compile_options,
        default_input_element_desc,
        std::size(default_input_element_desc));
    if (default_shader == nullptr)
        return -1;
    shader_array_buffer[SHADER_DEFAULT] = default_shader;

    // loading pixelart upscale shader
    D3D11_INPUT_ELEMENT_DESC upscale_input_element_desc[] = {
        {"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Shader* upscale_shader = CreateShader(
        device,
        memory,
        SHADER_UPSCALE,
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/win32/renderer/shaders_d3d11/upscale.hlsl",
        compile_options,
        upscale_input_element_desc,
        std::size(upscale_input_element_desc));
    if (upscale_shader == nullptr)
        return -1;
    shader_array_buffer[SHADER_UPSCALE] = upscale_shader;

    return 0;
}

/*
    NOTE(harsh): In D3D11 uniforms are CONSTANT BUFFERS

    Creates all the uniform buffers on the GPU.
    D3D11Buffer* for the created uniform buffers are stored on the uniform_buffers_array passed
    in.

    Returns S_OK on success, HRESULT error otherwise.
*/
HRESULT CreateAllUniformBuffers(
    ID3D11Device* device,
    ID3D11Buffer* (&uniform_buffers_array)[UNIFORM_BUFFER_COUNT])
{
    // ============== Create Per Frame Uniforms buffer ==============
    D3D11_BUFFER_DESC frame_uniform_buf_desc = {};
    frame_uniform_buf_desc.ByteWidth = sizeof(FrameUniforms);
    frame_uniform_buf_desc.Usage = D3D11_USAGE_DYNAMIC;
    frame_uniform_buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    frame_uniform_buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT result = device->CreateBuffer(
        &frame_uniform_buf_desc,
        NULL,
        &uniform_buffers_array[UNIFORM_PER_FRAME_BUFFER]);


    // ============== Create Per Entity Uniforms buffer ==============
    D3D11_BUFFER_DESC entity_uniform_buf_desc = {};
    entity_uniform_buf_desc.ByteWidth = sizeof(EntityUniforms);
    entity_uniform_buf_desc.Usage = D3D11_USAGE_DYNAMIC;
    entity_uniform_buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    entity_uniform_buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    result = device->CreateBuffer(
        &entity_uniform_buf_desc,
        NULL,
        &uniform_buffers_array[UNIFORM_PER_ENTITY_BUFFER]);

    return S_OK;
}

