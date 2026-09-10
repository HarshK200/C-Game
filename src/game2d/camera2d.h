#pragma once

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/game_math.h"

struct Camera2d
{
    Vec2 Position;
    float Zoom;
    float NearPlane;
    float FarPlane;

    /*
        TODO(harsh): add rotation and
        add an offest vec2 so when the view matrix is craeted teh camera can be offseted for
        smooth subpixel movement n sh*t
    */
};


Camera2d* Camera2dCreateAndInit(AppMemory* memory);
void Camera2dUpdate(Camera2d* camera);
Mat4 Camera2dGetViewMatrix(Camera2d* camera);
