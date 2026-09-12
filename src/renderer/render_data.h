#pragma once

#include "src/utils/enums.h"
#include "src/utils/game_math.h"


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


    // TODO(harsh): in future add a sortkey, albedo and albedo_override
};

struct RenderData
{
    RenderCommand* render_commands;
    int unsigned commands_count;
    int unsigned max_commands;

    ViewMatrixParams view_matrix_params;
    ProjectionMatrixParams projection_matrix_params;
};
