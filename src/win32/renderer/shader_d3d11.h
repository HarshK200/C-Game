#pragma once

#include <d3d11.h>
#include "src/main.h"

enum ShaderID
{
    Shader_Default = 0,
    Shader_Upscale = 1,
    Shader_Count // always the last gives the Shader* array size for free
};
struct Shader
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
};


Shader* CreateShader(
    Renderer* r,
    ArenaAllocator* permanent_allocator,
    ShaderID shader_id,
    const wchar_t* shader_file_path,
    UINT compile_options,
    D3D11_INPUT_ELEMENT_DESC* input_element_desc,
    UINT input_element_count);


int LoadAllShaders(Renderer* r, ArenaAllocator* permanent_allocator);
