// utils
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/game2d/camera2d.h"
#include "src/renderer/render_data.h"


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
