#pragma once

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"


struct SpriteSheet2d
{
    // TODO(harsh): mesh belongs in the sprite not the spritesheet
    MeshID MeshId;
    TextureID TextureId;
    Vec2i TextureScale;
};

struct Sprite2d
{
    SpriteSheet2d SpriteSheet;
    Vec2i TexelCoords;
    Vec2 Scale;
};

Vec4 GetSpriteUV(Sprite2d* sprite)
{
    Vec4 uv_min_max = {};
    uv_min_max.x = (float)sprite->TexelCoords.x / (float)sprite->SpriteSheet.TextureScale.x;
    uv_min_max.y = (float)sprite->TexelCoords.y / (float)sprite->SpriteSheet.TextureScale.y;
    uv_min_max.z = ((sprite->TexelCoords.x + sprite->Scale.x) - 1) / (float)sprite->SpriteSheet.TextureScale.x;
    uv_min_max.w = ((sprite->TexelCoords.y + sprite->Scale.y) - 1) / (float)sprite->SpriteSheet.TextureScale.y;

    return uv_min_max;
}
