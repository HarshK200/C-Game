#pragma once

#include <d3d11.h>


// ====================================== TEMP START ======================================
// TODO(harsh): Temporarly placed here, move these structs to there
// dedicated file, like shader_d3d11.cpp, mesh_d3d11.h and triangle_mesh_d3d11.cpp
// which includes the mesh_d3d11.h file for Mesh struct definition
enum ShaderID
{
    Shader_Default = 0,
    Shader_Blit = 1,
    Shader_Count // always the last gives the Shader* array size for free
};
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
// ====================================== TEMP END ======================================


struct Renderer
{
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;

    ID3D11RenderTargetView* BackBufferRTV;
    ID3D11Texture2D* InternalRenderTexture; // 640x360 i.e. 16:9 aspect ratio
    ID3D11RenderTargetView* InternalRTV;
    ID3D11ShaderResourceView* InternalSRV;

    Shader* Shaders[Shader_Count];
    Mesh* TriangleMesh;
};
