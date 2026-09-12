// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/constants.h"

#include "src/renderer/render_data.h"

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
