#pragma once

// utils
#include "src/utils/game_math.h"

#include "src/game2d/sprite2d.h"
#include "src/renderer/render_data.h"


struct Player
{
    Vec2 position;
    Vec2 scale;
    Sprite2d sprite;
};
Player* PlayerCreateAndInit(AppMemory* memory);
void PlayerUpdate(Player* player, RenderData* render_data);
