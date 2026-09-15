#pragma once

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/game2d/sprite2d.cpp"
#include "src/renderer/render_data.cpp"


struct Player
{
    Vec2 position;
    Vec2 size;
    Sprite2d sprite;
};

/*
    NOTE(harsh): allocates using PermanentAllocator
*/
Player* PlayerCreateAndInit(AppMemory* memory)
{
    Player* player = ArenaAlloc<Player>(&memory->PermanentAllocator, sizeof(Player));
    player->position = {0.0f, 0.0f};
    player->size = {32.0f, 48.0f};
    player->sprite = {
        {
            MESH_QUAD,
            TEXTURE_ENTITY_ATLAS,
            {96, 48},
        },
        {0, 0},
        player->size,
    };

    return player;
}

/*
    updates the player state and set the render_data.command required for rendering
*/
void PlayerUpdateAndPushRender(AppMemory* memory, Player* player, RenderData* render_data)
{
    LOG_ASSERT((render_data->commands_count + 1) < render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    RenderCommand render_command = {};
    render_command.mesh_id = player->sprite.sprite_sheet.mesh_id;
    render_command.texture_id = player->sprite.sprite_sheet.texture_id;

    render_command.transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    render_command.transforms[0] = ModelMat4(player->position, player->size);

    render_command.uv_min_max = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * 1);
    render_command.uv_min_max[0] = GetSpriteUV(&player->sprite);

    render_command.instanced = false;

    PushRenderCommand(render_data, render_command);
}
