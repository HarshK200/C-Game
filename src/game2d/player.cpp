// utils
#include "sprite2d.h"
#include "src/utils/enums.h"
#include "src/utils/arena_allocator.h"
#include "src/utils/game_math.h"
#include "src/utils/log.h"

#include "src/game2d/player.h"
#include "src/renderer/render_data.h"


/*
    NOTE(harsh): allocates using PermanentAllocator
*/
Player* PlayerCreateAndInit(AppMemory* memory)
{
    Player* player = (Player*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Player));
    player->position = {0.0f, 0.0f};
    player->scale = {32.0f, 48.0f};
    player->sprite = {
        {
            MESH_QUAD,
            TEXTURE_ENTITY_ATLAS,
            {96, 48},
        },
        {0, 0},
        player->scale,
    };

    return player;
}

/*
    updates the player state and set the render_data.command required for rendering
*/
void PlayerUpdate(Player* player, RenderData* render_data)
{
    LOG_ASSERT((render_data->commands_count + 1) < render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    RenderCommand render_command = {};
    render_command.mesh_id = player->sprite.sprite_sheet.mesh_id;
    render_command.texture_id = player->sprite.sprite_sheet.texture_id;
    render_command.transform = ModelMat4(player->position, player->scale);
    render_command.uv_min_max = GetSpriteUV(&player->sprite);

    PushRenderCommand(render_data, render_command);
}
