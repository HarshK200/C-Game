#pragma once

// utils
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.cpp"


struct Camera2d
{
    Vec2 PrevPosition;
    Vec2 Position;
    Vec2 Offset;
    float Zoom;
    float NearPlane;
    float FarPlane;

    float Speed;

    /*
        TODO(harsh): add rotation and add an offest vec2 so when the view matrix is craeted teh camera can be offseted for
        smooth subpixel movement n sh*t
    */
};


Camera2d* Camera2dCreateAndInit(AppMemory* memory)
{
    Camera2d* camera = ArenaAlloc<Camera2d>(&memory->PermanentAllocator, sizeof(Camera2d));
    camera->Position = {0, 0};
    camera->Speed = 5.0f;
    camera->Offset = {INTERNAL_RENDER_RESOLUTION.x / 2, INTERNAL_RENDER_RESOLUTION.y / 2};
    camera->Zoom = 1;
    // TODO(harsh): how does this whole near and far plane things work? and is -1 near_plane
    // correct for the ZO i.e. the d3d11 render z axis thinig?? in the Orthographic_ZO_RH matrix?
    camera->NearPlane = -1;
    camera->FarPlane = 1;

    return camera;
}

void Camera2dPhysicsUpdate(Camera2d* camera, double delta_time, Vec2 player_pos)
{
    camera->PrevPosition = camera->Position;
    camera->Position = LerpVec2(camera->Position, player_pos, camera->Speed * delta_time);
}

void Camera2dUpdate()
{
}


void Camera2dQueueRender(Camera2d* camera, double interpolation_alpha, RenderData* render_data)
{
    // Update Render Data
    render_data->view_matrix_params.Offset = camera->Offset;
    render_data->view_matrix_params.Position = LerpVec2(camera->PrevPosition, camera->Position, interpolation_alpha);
    // TODO(harsh): disable zooming logic on release build
    render_data->view_matrix_params.Zoom = camera->Zoom;
    render_data->projection_matrix_params.NearPlane = camera->NearPlane;
    render_data->projection_matrix_params.FarPlane = camera->FarPlane;
}
