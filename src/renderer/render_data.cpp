#pragma once

// utils
#include "src/utils/enums.h"
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"


struct RenderCommand
{
    MeshID mesh_id;
    TextureID texture_id;
    Mat4* transform;
    Vec4* uv_min_max;

    int no_of_instances; // no_of_instances should be 0 if instanced is false
    bool instanced;      // should be false if drawing only one entity

    // TODO(harsh): in future add a sortkey, albedo and albedo_override
};

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

struct RenderData
{
    RenderCommand* render_commands; // array of RenderCommand* allocated using temp_arena_allocator
    int unsigned commands_count;
    int unsigned max_commands;

    ViewMatrixParams view_matrix_params;
    ProjectionMatrixParams projection_matrix_params;
};


RenderData* CreateFrameRenderData(ArenaAllocator* temp_arena_allocator)
{
    RenderData* render_data = (RenderData*)ArenaAlloc(
        temp_arena_allocator,
        sizeof(RenderData));

    // NOTE(harsh): allocating space for 100 render commands per frame. *FOR NOW, MIGHT CHANGE LATER*
    render_data->render_commands = (RenderCommand*)ArenaAlloc(
        temp_arena_allocator,
        sizeof(RenderCommand) * MAX_RENDER_COMMANDS_PER_FRAME);
    render_data->max_commands = MAX_RENDER_COMMANDS_PER_FRAME;

    return render_data;
}

int PushRenderCommand(RenderData* render_data, RenderCommand render_command)
{
    LOG_ASSERT((render_data->commands_count + 1) <= render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    render_data->render_commands[render_data->commands_count] = render_command;
    render_data->commands_count += 1;

    return 0;
}
