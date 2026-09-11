#pragma once

#include <d3d11.h>
#include <winnt.h>

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/globals.h"

struct Shader
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
};


Shader* CreateShader(
    ID3D11Device* device,
    AppMemory* memory,
    ShaderID shader_id,
    const wchar_t* shader_file_path,
    UINT compile_options,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT input_element_count);


int LoadAllShaders(
    AppMemory* memory,
    ID3D11Device* device,
    Shader* (&shader_array_buffer)[SHADER_COUNT]);


// Uniform Buffers Layouts
struct FrameUniforms
{
    Mat4 View;
    Mat4 Projection;
};
struct EntityUniforms
{
    Mat4 Model;
    // TODO(harsh): maybe add a AlbedoOverride in the future for damage taken flashs
};


HRESULT CreateAllUniformBuffers(
    ID3D11Device* device,
    ID3D11Buffer* (&uniform_buffers_array)[UNIFORM_BUFFER_COUNT]);

template <typename T>
HRESULT UploadUniformBufferData(
    ID3D11DeviceContext* device_context,
    ID3D11Buffer* uniform_buffer,
    T& uniform_data);
