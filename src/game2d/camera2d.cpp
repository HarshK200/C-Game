#pragma once

// utils
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.cpp"


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


Camera2d* Camera2dCreateAndInit(AppMemory* memory)
{
    Camera2d* camera = (Camera2d*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Camera2d));
    camera->position = {0, 0};
    camera->offset = {INTERNAL_RENDER_RESOLUTION.x / 2, INTERNAL_RENDER_RESOLUTION.y / 2};
    camera->zoom = 1;
    // TODO(harsh): how does this whole near and far plane things work? and is -1 near_plane
    // correct for the ZO i.e. the d3d11 render z axis thinig?? in the Orthographic_ZO_RH matrix?
    camera->near_plane = -1;
    camera->far_plane = 1;

    return camera;
}


void Camera2dUpdate(Camera2d* camera, RenderData* render_data)
{
    // Update Render Data
    render_data->view_matrix_params.offset = camera->offset;
    render_data->view_matrix_params.position = camera->position;
    // TODO(harsh): disable zooming logic on release build
    render_data->view_matrix_params.zoom = camera->zoom;
    render_data->projection_matrix_params.near_plane = camera->near_plane;
    render_data->projection_matrix_params.far_plane = camera->far_plane;
}
