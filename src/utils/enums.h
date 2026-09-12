#pragma once


enum TextureID
{
    TEXTURE_ENTITY_ATLAS = 0,
    TEXTURE_TILEMAP_ATLAS = 1,
    TEXTURE_COUNT
};

enum MeshID
{
    MESH_TRIANGLE = 0,
    MESH_QUAD = 1,
    MESH_UPSCALE_QUAD = 2,
    MESH_COUNT
};

enum ShaderID
{
    SHADER_DEFAULT = 0,
    SHADER_UPSCALE = 1,
    SHADER_COUNT // always the last gives the Shader* array size for free
};

enum UniformBufferID
{
    UNIFORM_PER_FRAME_BUFFER = 0,
    UNIFORM_PER_ENTITY_BUFFER = 1,
    UNIFORM_BUFFER_COUNT
};
