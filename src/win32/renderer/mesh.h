#pragma once
#include <d3d11.h>


struct Mesh
{
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    UINT VertexCount;
    UINT VertexStride;
    UINT VertexOffset;
    UINT IndexCount;
    UINT IndexOffset;
};


Mesh* CreateUpscaleQuadMesh(ID3D11Device* device);
Mesh* CreateTriangleMesh(ID3D11Device* device);
Mesh* CreateQuadMesh(ID3D11Device* device);
