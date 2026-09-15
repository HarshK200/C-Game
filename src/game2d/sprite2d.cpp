#pragma once

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"


struct SpriteSheet2d
{
    // TODO(harsh): mesh belongs in the sprite not the spritesheet
    MeshID mesh_id;
    TextureID texture_id;
    Vec2i texture_size;
};

struct Sprite2d
{
    SpriteSheet2d sprite_sheet;
    Vec2i texel_coords;
    Vec2 size;
};

Vec4 GetSpriteUV(Sprite2d* sprite)
{
    Vec2 uv_min = {};
    uv_min.x = (float)sprite->texel_coords.x / (float)sprite->sprite_sheet.texture_size.x;
    uv_min.y = (float)sprite->texel_coords.y / (float)sprite->sprite_sheet.texture_size.y;

    Vec2 uv_max = {};
    uv_max.x = ((sprite->texel_coords.x + sprite->size.x) - 1) / (float)sprite->sprite_sheet.texture_size.x;
    uv_max.y = ((sprite->texel_coords.y + sprite->size.y) - 1) / (float)sprite->sprite_sheet.texture_size.y;

    Vec4 uv_min_max = {uv_min.x, uv_min.y, uv_max.x, uv_max.y};

    return uv_min_max;
}
