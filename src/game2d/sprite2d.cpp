#pragma once

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"


struct SpriteSheet2d
{
    MeshID MeshId;
    TextureID TextureId;

    // TODO(harsh): Add a Sprite-offset
};

struct Sprite2d
{
    SpriteSheet2d SpriteSheet;
    Vec2i TexelCoords;
    Vec2i Scale;
    Vec2 PosOffset;
};
