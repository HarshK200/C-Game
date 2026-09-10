#pragma once

#include "src/utils/game_math.h"
#include "src/game2d/sprite2d.h"
#include "src/utils/arena_allocator.h"

struct Player
{
    Vec2 Position;
    Vec2i Scale;
    float MoveSpeed;
    Sprite2D Sprite;
};

Player* PlayerCreateAndInit(AppMemory* memory);
void PlayerUpdate(AppMemory* memory);
