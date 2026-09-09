#include <d3d11.h>
#include <iterator>

#include "src/win32/renderer/mesh.h"

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"


/*
    NOTE(harsh): Y+ is considered up,
    The Upscale Quad Mesh *MUST NOT* be used anywhere other than during the
    RenderPass_Upscale.


    Uploads Upscale quad mesh vertex & index buffer to the GPU,
    Returns the resulting pointer on success, otherwise returns a nullptr on failure

    TODO(harsh): The Vertex Data follows Y+ Up for the type being, Change it to Y+ Down
    for ease and consistency and the ProjectionMatrix will then flip it
*/
Mesh* CreateUpscaleQuadMesh(ID3D11Device* device, AppMemory* memory)
{
    Mesh* upscale_quad_mesh = (Mesh*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Mesh));
    // vertex buffer data
    // clang-format off
    float vertex_buffer_data[] = {
         // position        // uv
         1.0f,  1.0f,       1.0f, 0.0f,   // top-right
         1.0f, -1.0f,       1.0f, 1.0f,   // bottom-right
        -1.0f,  1.0f,       0.0f, 0.0f,   // top-left
        -1.0f, -1.0f,       0.0f, 1.0f,   // bottom-left
    };

    // index buffer data
    // NOTE(harsh): the indices must be in clockwise order for each triangle otherwise it won't be drawn
    // because of back culling.
    int unsigned index_buffer_data[] = {
        2, 0, 1, // top half triangle
        2, 1, 3, // bottom half triangle
    };
    // clang-format on
    upscale_quad_mesh->VertexBuffer = nullptr;

    // set vertex buffer info
    upscale_quad_mesh->VertexStride = sizeof(float) * 4;
    upscale_quad_mesh->VertexCount = 4;
    upscale_quad_mesh->VertexOffset = 0;

    // upload vertex buffer
    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.ByteWidth = sizeof(vertex_buffer_data);
    vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertex_sr_data = {};
    vertex_sr_data.pSysMem = vertex_buffer_data;
    HRESULT result = device->CreateBuffer(&vertex_buffer_desc, &vertex_sr_data, &upscale_quad_mesh->VertexBuffer);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 Quad Vertex Buffer creation FAILED! with error code: %d", result);
        return nullptr;
    }

    // set index buffer info
    upscale_quad_mesh->IndexCount = std::size(index_buffer_data);
    upscale_quad_mesh->IndexOffset = 0;

    // upload index buffer
    D3D11_BUFFER_DESC index_buffer_desc = {};
    index_buffer_desc.ByteWidth = sizeof(index_buffer_data);
    index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA index_sr_data = {};
    index_sr_data.pSysMem = index_buffer_data;
    result = device->CreateBuffer(&index_buffer_desc, &index_sr_data, &upscale_quad_mesh->IndexBuffer);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 Quad Index Buffer creation FAILED! with error code: %d", result);
        return nullptr;
    }


    return upscale_quad_mesh;
}

/*
    NOTE(harsh): triangle mesh does not use index buffers,
    therefore DrawIndexed() should not be called with a triangle_mesh bound.

    Uploads triangle mesh vertex buffer to the GPU.
    Returns the resulting pointer on success, otherwise returns a nullptr on failure

    TODO(harsh): The Vertex Data follows Y+ Up, No Matrix multiplication happens here
    but do come back and have a look at the way vertex are position if they are correct or not
*/
Mesh* CreateTriangleMesh(ID3D11Device* device, AppMemory* memory)
{
    Mesh* triangle_mesh = (Mesh*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Mesh));

    // vertex buffer data
    // clang-format off
    float vertex_buffer_data[] = {
         // position            // uv
         0.0f,  0.5f, 0.0f,     0.5, 0.0,       // top-middle
         0.5f, -0.5f, 0.0f,     1.0, 1.0,       // bottom-right
        -0.5f, -0.5f, 0.0f,     0.0, 1.0,       // bottom-left
    };
    // clang-format on
    triangle_mesh->VertexBuffer = nullptr;

    // set vertex buffer info
    triangle_mesh->VertexStride = sizeof(float) * 5;
    triangle_mesh->VertexCount = 3;
    triangle_mesh->VertexOffset = 0;

    // upload vertex buffer
    D3D11_BUFFER_DESC vertex_buff_desc = {};
    vertex_buff_desc.ByteWidth = sizeof(vertex_buffer_data);
    vertex_buff_desc.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertex_sr_data = {};
    vertex_sr_data.pSysMem = vertex_buffer_data;
    HRESULT result = device->CreateBuffer(
        &vertex_buff_desc,
        &vertex_sr_data,
        &triangle_mesh->VertexBuffer);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 Triangle Vertex Buffer creation FAILED! with error code: %d", result);
        return nullptr;
    }

    return triangle_mesh;
}

/*
    Uploads quad mesh vertex & index buffer to the GPU,
    Returns the resulting pointer on success, otherwise returns a nullptr on failure


    TODO(harsh): The Vertex Data follows Y+ Up for the type being, Change it to Y+ Down
    for ease and consistency and the ProjectionMatrix will then flip it
*/
Mesh* CreateQuadMesh(ID3D11Device* device, AppMemory* memory)
{
    Mesh* quad_mesh = (Mesh*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Mesh));

    // vertex buffer data
    // clang-format off
    float vertex_buffer_data[] = {
         // position            // uv
         0.5f,  0.5f, 0.0f,     1.0f, 0.0f,   // top-right
         0.5f, -0.5f, 0.0f,     1.0f, 1.0f,   // bottom-right
        -0.5f,  0.5f, 0.0f,     0.0f, 0.0f,   // top-left
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f,   // bottom-left
    };

    // index buffer data
    // NOTE(harsh): the indices must be in clockwise order for each triangle otherwise it won't be drawn
    // because of back culling.
    int unsigned index_buffer_data[] = {
        2, 0, 1, // top half triangle
        3, 2, 1, // bottom half triangle
    };
    // clang-format on
    quad_mesh->VertexBuffer = nullptr;

    // set vertex buffer info
    quad_mesh->VertexStride = sizeof(float) * 5;
    quad_mesh->VertexCount = 4;
    quad_mesh->VertexOffset = 0;

    // upload vertex buffer
    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.ByteWidth = sizeof(vertex_buffer_data);
    vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertex_sr_data = {};
    vertex_sr_data.pSysMem = vertex_buffer_data;
    HRESULT result = device->CreateBuffer(&vertex_buffer_desc, &vertex_sr_data, &quad_mesh->VertexBuffer);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 Quad Vertex Buffer creation FAILED! with error code: %d", result);
        return nullptr;
    }

    // set index buffer info
    quad_mesh->IndexCount = std::size(index_buffer_data);
    quad_mesh->IndexOffset = 0;

    // upload index buffer
    D3D11_BUFFER_DESC index_buffer_desc = {};
    index_buffer_desc.ByteWidth = sizeof(index_buffer_data);
    index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA index_sr_data = {};
    index_sr_data.pSysMem = index_buffer_data;
    result = device->CreateBuffer(&index_buffer_desc, &index_sr_data, &quad_mesh->IndexBuffer);
    if (FAILED(result))
    {
        LOG_ERRORF("D3D11 Quad Index Buffer creation FAILED! with error code: %d", result);
        return nullptr;
    }

    return quad_mesh;
}
