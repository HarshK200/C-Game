// utils
#include "src/utils/arena_allocator.h"

#include "src/game/camera2d.h"


Camera2D* CameraCreateAndInit(AppMemory* memory)
{
    Camera2D* camera = (Camera2D*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Camera2D));
    camera->position = {0, 0};
    camera->zoom = 1;
    // TODO(harsh): how does this whole near and far plane things work? and is -1 near_plane
    // correct for the ZO i.e. the d3d11 render z axis thinig?? in the Orthographic_ZO_RH matrix?
    camera->near_plane = -1;
    camera->far_plane = 1;


    return camera;
}


void CameraUpdate(Camera2D* camera)
{
}
