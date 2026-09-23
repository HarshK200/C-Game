#pragma once

// utils
#include "src/utils/enums.h"
#include "src/utils/constants.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"


enum LayerID
{
    LAYER_BACKGROUND = 0,
    LAYER_FOREGROUND = 1,
    LAYER_UI = 2,
    LAYER_COUNT,
};

struct RenderCommand
{
    MeshID MeshId;
    TextureID TextureId;
    Mat4* Transforms;
    Vec2i* SpriteCoords;
    Vec2i* SpriteScale;

    int NoOfInstances; // no_of_instances should be 0 if instanced is false
    bool Instanced;    // should be false if drawing only one entity

    LayerID LayerId;
    int SortOrder;

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
    RenderCommand* RenderCommands; // array of RenderCommand allocated using temp_arena_allocator
    int unsigned RenderCommandsCount;
    int unsigned MaxCommands;

    ViewMatrixParams ViewMatParams;
    ProjectionMatrixParams ProjectionMatParams;
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
    render_data->MaxCommands = MAX_RENDER_COMMANDS_PER_FRAME;

    return render_data;
}

inline int PushRenderCommand(RenderData* render_data, RenderCommand* render_command)
{
    LOG_ASSERT((render_data->RenderCommandsCount + 1) <= render_data->MaxCommands, "Maximum render commands per frame reached! cannot push more render commands");
    LOG_ASSERT((render_command->NoOfInstances < MAX_INSTANCE_BUFFER_SIZE), "Invalid render_command, no of MAX_INSTANCE_BUFFER_SIZE exceeded");

    render_data->RenderCommands[render_data->RenderCommandsCount] = *render_command;
    render_data->RenderCommandsCount += 1;

    return 0;
}

inline int CalculateSortOrder(Vec2 position)
{
    return (int)position.y * 100;
}
