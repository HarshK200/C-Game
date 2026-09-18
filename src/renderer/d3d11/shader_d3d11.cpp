#include "src/pch.h"

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/file_io.h"
#include "src/utils/game_math.h"
#include "src/utils/constants.h"
#include "src/utils/arena_allocator.h"


struct Shader
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    ID3D11Buffer* InstanceBuffer;
    UINT InstanceBufferStride;
    UINT InstanceBufferOffset;
};


// Uniform Buffers Layouts
struct FrameUniforms
{
    Mat4 View;
    Mat4 Projection;
};
struct EntityData
{
    Mat4 Model;
    Vec4 UVMinMax;
    // TODO(harsh): maybe add a AlbedoOverride in the future for damage taken flashs
};


/*
    Compiles the vertex and pixel shaders from shader_file_path using compile_options.
    Sets the resulting shader in the Renderer's Shaders array.
    if input_element_desc != nullptr then the InputLayout for the shader is also created.
    if instance_buffer_size > 0 then creates the instance_buffer
    WARNING: the input_element_count must be > 0 when input_element_desc is passed in.

    Returns Shader* on success, otherwise returns nullptr on failure.
*/
Shader* CreateShader(
    ID3D11Device* device,
    AppMemory* memory,
    ShaderID shader_id,
    const wchar_t* shader_file_path,
    UINT compile_options,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT input_element_count,
    UINT instance_buffer_size,
    UINT instance_buffer_stride)
{
    ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *error_blob = nullptr;
    Shader* shader = ArenaAlloc<Shader>(&memory->PermanentAllocator, sizeof(Shader));

    // check if the shader file exists or not
    if (!GameFileIO::FileExists(shader_file_path))
    {
        LOG_ERRORF("Shader file not found at path: %ls", shader_file_path);
        return nullptr;
    }

    HRESULT result;

    /*
        TODO(harsh): cache compiled shader on first creation and if CreateShader() gets
        called and cached compiled shader exists just return that instead
    */
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


        // create instance buffer for the shader if required
        if (instance_buffer_size > 0)
        {
            D3D11_BUFFER_DESC instance_buffer_desc = {};
            instance_buffer_desc.ByteWidth = instance_buffer_size;
            instance_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
            instance_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            instance_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            HRESULT result = device->CreateBuffer(&instance_buffer_desc, NULL, &shader->InstanceBuffer);
            if (FAILED(result))
                goto cleanup;
            shader->InstanceBufferStride = instance_buffer_stride;
            shader->InstanceBufferOffset = 0;
        }
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

        {"MODEL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MODEL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MODEL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MODEL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"UV_MIN_MAX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };
    UINT instance_buffer_size = sizeof(EntityData) * MAX_INSTANCE_BUFFER_SIZE;
    UINT instance_buffer_stride = sizeof(EntityData);
    Shader* default_shader = CreateShader(
        device,
        memory,
        SHADER_DEFAULT,
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/renderer/d3d11/shaders/default.hlsl",
        compile_options,
        default_input_element_desc,
        std::size(default_input_element_desc),
        instance_buffer_size,
        instance_buffer_stride);
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
        L"C:/Users/Harsh/Desktop/personal_dev/cpp_game/src/renderer/d3d11/shaders/upscale.hlsl",
        compile_options,
        upscale_input_element_desc,
        std::size(upscale_input_element_desc),
        0,
        0);
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
    if (FAILED(result))
        return result;

    return result;
}

template <typename T>
HRESULT UploadUniformBufferData(
    ID3D11DeviceContext* device_context,
    ID3D11Buffer* uniform_buffer,
    T& uniform_data)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT result = device_context->Map(
        uniform_buffer,
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped);
    if (FAILED(result))
        return result;

    memcpy(mapped.pData, &uniform_data, sizeof(T));

    device_context->Unmap(uniform_buffer, 0);

    return result;
}


// TODO(harsh): implement this
template <typename T>
HRESULT UploadInstanceBufferData(
    ID3D11DeviceContext* device_context,
    ID3D11Buffer* instance_buffer,
    T* instance_data,
    UINT instance_count)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT result = device_context->Map(
        instance_buffer,
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped);
    if (FAILED(result))
        return result;

    memcpy(mapped.pData, instance_data, sizeof(T) * instance_count);

    device_context->Unmap(instance_buffer, 0);

    return result;
}
