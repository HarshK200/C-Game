#pragma once
#include <d3d11.h>

#include "src/utils/arena_allocator.h"

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


Mesh* CreateUpscaleQuadMesh(ID3D11Device* device, ArenaAllocator* permanent_allocator);
Mesh* CreateTriangleMesh(ID3D11Device* device, ArenaAllocator* permanent_allocator);
Mesh* CreateQuadMesh(ID3D11Device* device, ArenaAllocator* permanent_allocator);
