#pragma once

#include "src/utils/game_math.h"
#include "src/utils/globals.h"

#include "src/win32/renderer/mesh.h"
#include "src/win32/renderer/shader_d3d11.h"
#include "src/win32/renderer/texture_d3d11.h"


struct Renderer
{
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;

    ID3D11RenderTargetView* BackBufferRTV;
    ID3D11Texture2D* InternalRenderTexture; // 640x360 i.e. 16:9 aspect ratio
    ID3D11RenderTargetView* InternalRTV;
    ID3D11ShaderResourceView* InternalSRV;

    Shader* Shaders[SHADER_COUNT];
    Texture2D* Textures[TEXTURE_COUNT];
    ID3D11SamplerState* PointSampler; // TODO(harsh): maybe create a ID3D11SamplerState* array like the shader arary?

    Mesh* UpscaleQuadMesh;
    Mesh* TriangleMesh;
    Mesh* QuadMesh;
};

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
