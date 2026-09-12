#pragma once

#include <d3d11.h>

// utils
#include "src/utils/enums.h"
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


Mesh* CreateUpscaleQuadMesh(ID3D11Device* device, AppMemory* memory);
Mesh* CreateTriangleMesh(ID3D11Device* device, AppMemory* memory);
Mesh* CreateQuadMesh(ID3D11Device* device, AppMemory* memory);
int CreateAllMeshs(
    AppMemory* memory,
    ID3D11Device* device,
    Mesh* (&mesh_array_buffer)[MESH_COUNT]);
