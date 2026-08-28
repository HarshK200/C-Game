#include <Windows.h>
#include <d3d11.h>

#include "../main.h"
#include "win32_platform.h"


void RendererInit(PlatformWindow Window)
{
    PlatformPrintDebug("Renderer Init\n");

    // SimpleVertex VertexBuffer[] = {
    //     {0.5f, 0.5f, 0.0f},
    //     {-0.5f, -0.5f, 0.0f},
    //     {0.5f, -0.5f, 0.0f},
    // };
    //
    // D3D11_BUFFER_DESC BufferDesc = {};
    // BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    // BufferDesc.ByteWidth = sizeof(Vec3) * 3;
    // BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // BufferDesc.CPUAccessFlags = 0;
    // BufferDesc.MiscFlags = 0;
    //
    // D3D11_SUBRESOURCE_DATA InitData;
    // InitData.pSysMem = VertexBuffer;
    // InitData.SysMemPitch = 0;
    // InitData.SysMemSlicePitch = 0;
    //
    // ID3D11Buffer* vertexBuffer;
    // create the vertex buffer
    // TODO(harsh): get that global d3d11_device to call create buffer
    // HRESULT result = Device->CreateBuffer();
}

void RendererUpdate() {}
