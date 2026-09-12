#pragma once

// utils
#include "src/utils/enums.h"
#include "src/utils/game_math.h"


struct SpriteSheet2d
{
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

Vec4 GetSpriteUV(Sprite2d* sprite);
