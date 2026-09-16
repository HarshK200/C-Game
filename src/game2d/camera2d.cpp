#pragma once

// utils
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.cpp"


struct Camera2d
{
    Vec2 Position;
    Vec2 Offset;
    float Zoom;
    float NearPlane;
    float FarPlane;

    /*
        TODO(harsh): add rotation and add an offest vec2 so when the view matrix is craeted teh camera can be offseted for
        smooth subpixel movement n sh*t
    */
};


Camera2d* Camera2dCreateAndInit(AppMemory* memory)
{
    Camera2d* camera = ArenaAlloc<Camera2d>(&memory->PermanentAllocator, sizeof(Camera2d));
    camera->Position = {0, 0};
    camera->Offset = {INTERNAL_RENDER_RESOLUTION.x / 2, INTERNAL_RENDER_RESOLUTION.y / 2};
    camera->Zoom = 1;
    // TODO(harsh): how does this whole near and far plane things work? and is -1 near_plane
    // correct for the ZO i.e. the d3d11 render z axis thinig?? in the Orthographic_ZO_RH matrix?
    camera->NearPlane = -1;
    camera->FarPlane = 1;

    return camera;
}


void Camera2dUpdate(Camera2d* camera, RenderData* render_data)
{
    // Update Render Data
    render_data->view_matrix_params.Offset = camera->Offset;
    render_data->view_matrix_params.Position = camera->Position;
    // TODO(harsh): disable zooming logic on release build
    render_data->view_matrix_params.Zoom = camera->Zoom;
    render_data->projection_matrix_params.NearPlane = camera->NearPlane;
    render_data->projection_matrix_params.FarPlane = camera->FarPlane;
}
