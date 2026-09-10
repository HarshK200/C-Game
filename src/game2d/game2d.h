#pragma once

#include "src/game2d/player/player.h"
#include "src/game2d/camera2d.h"

// Struct Definitions Only
struct Game2d
{
    Camera2d* Camera;
    Player* Player;
};
