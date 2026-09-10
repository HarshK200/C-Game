// utils
#include "src/utils/globals.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"

#include "src/game2d/camera2d.h"


Camera2d* Camera2dCreateAndInit(AppMemory* memory)
{
    Camera2d* camera = (Camera2d*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Camera2d));
    camera->position = {0, 0};
    camera->zoom = 1;
    // TODO(harsh): how does this whole near and far plane things work? and is -1 near_plane
    // correct for the ZO i.e. the d3d11 render z axis thinig?? in the Orthographic_ZO_RH matrix?
    camera->near_plane = -1;
    camera->far_plane = 1;


    return camera;
}


void Camera2dUpdate(Camera2d* camera)
{
    // TODO(harsh): disable zooming logic on release build
}

Mat4 Camera2dGetViewMatrix(Camera2d* camera)
{
    LOG_ASSERT(camera, "Camera is nullptr");
    const Vec2 CAMERA_CENTER_OFFSET = {
        (-INTERNAL_RENDER_RESOLUTION.x / 2),
        (-INTERNAL_RENDER_RESOLUTION.y / 2),
    };

    Mat4 view_matrix = Translate_Mat4({CAMERA_CENTER_OFFSET.x, CAMERA_CENTER_OFFSET.y, 0.0f});
    view_matrix = Mat4xMat4(Scale_Mat4({camera->zoom, camera->zoom, 1.0f}), view_matrix);
    view_matrix = Mat4xMat4(Translate_Mat4({-camera->position.x, -camera->position.y, 0.0f}), view_matrix);

    return view_matrix;
}
