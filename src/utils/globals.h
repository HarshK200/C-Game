#pragma once

#include "game_math.h"


// constants
inline constexpr Vec2 INTERNAL_RENDER_RESOLUTION = {640.0f, 360.0f};
inline constexpr Vec2 DEFAULT_WINDOW_RESOLUTION = {1280.0f, 720.0f};


// enums
enum TextureID
{
    TEXTURE_ENTITY_ATLAS = 0,
    TEXTURE_TILEMAP_ATLAS = 1,
    TEXTURE_COUNT
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
