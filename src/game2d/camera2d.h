#pragma once

// utils
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.h"

struct Camera2d
{
    Vec2 position;
    Vec2 offset;
    float zoom;
    float near_plane;
    float far_plane;

    /*
        TODO(harsh): add rotation and add an offest vec2 so when the view matrix is craeted teh camera can be offseted for
        smooth subpixel movement n sh*t
    */
};


Camera2d* Camera2dCreateAndInit(AppMemory* memory);
void Camera2dUpdate(Camera2d* camera, RenderData* render_data);
