#pragma once

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/game_math.h"

struct Camera2d
{
    Vec2 position;
    float zoom;
    float near_plane;
    float far_plane;

    /*
        TODO(harsh): add rotation and
        add an offest vec2 so when the view matrix is craeted teh camera can be offseted for
        smooth subpixel movement n sh*t
    */
};


Camera2d* CameraCreateAndInit(AppMemory* memory);
void CameraUpdate(Camera2d* camera);
