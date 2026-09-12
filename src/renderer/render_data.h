#pragma once

// utils
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"


struct ViewMatrixParams
{
    Vec2 position;
    Vec2 offset;
    float zoom;
};
struct ProjectionMatrixParams
{
    float near_plane;
    float far_plane;
};

struct RenderCommand
{
    MeshID mesh_id;
    TextureID texture_id;
    Mat4 transform; // this just the model matrix
    Vec4 uv_min_max;

    // TODO(harsh): in future add a sortkey, albedo and albedo_override
};

struct RenderData
{
    RenderCommand* render_commands; // array of RenderCommand* allocated using temp_arena_allocator
    int unsigned commands_count;
    int unsigned max_commands;

    ViewMatrixParams view_matrix_params;
    ProjectionMatrixParams projection_matrix_params;
};

RenderData* CreateFrameRenderData(ArenaAllocator* temp_arena_allocator);
int PushRenderCommand(RenderData* render_data, RenderCommand render_command);
