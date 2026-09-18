#pragma once

// utils
#include "src/utils/enums.h"
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"


enum LayerID
{
};

struct RenderCommand
{
    MeshID MeshId;
    TextureID TextureId;
    Mat4* Transforms;
    Vec4* UvMinMax;

    int NoOfInstances; // no_of_instances should be 0 if instanced is false
    bool Instanced;    // should be false if drawing only one entity

    // TODO(harsh): implemente this layer in renderer
    LayerID LayerId;

    // TODO(harsh): albedo and albedo_override
};

struct ViewMatrixParams
{
    Vec2 Position;
    Vec2 Offset;
    float Zoom;
};
struct ProjectionMatrixParams
{
    float NearPlane;
    float FarPlane;
};

struct RenderData
{
    RenderCommand* RenderCommands; // array of RenderCommand* allocated using temp_arena_allocator
    int unsigned commands_count;
    int unsigned max_commands;

    ViewMatrixParams view_matrix_params;
    ProjectionMatrixParams projection_matrix_params;
};


inline RenderData* CreateFrameRenderData(ArenaAllocator* temp_arena_allocator)
{
    RenderData* render_data = ArenaAlloc<RenderData>(
        temp_arena_allocator,
        sizeof(RenderData));

    // NOTE(harsh): allocating space for 100 render commands per frame. *FOR NOW, MIGHT CHANGE LATER*
    render_data->RenderCommands = ArenaAlloc<RenderCommand>(
        temp_arena_allocator,
        sizeof(RenderCommand) * MAX_RENDER_COMMANDS_PER_FRAME);
    render_data->max_commands = MAX_RENDER_COMMANDS_PER_FRAME;

    return render_data;
}

inline int PushRenderCommand(RenderData* render_data, RenderCommand* render_command)
{
    LOG_ASSERT((render_data->commands_count + 1) <= render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    render_data->RenderCommands[render_data->commands_count] = *render_command;
    render_data->commands_count += 1;

    return 0;
}
