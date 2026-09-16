#pragma once

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"
#include "src/game2d/sprite2d.cpp"
#include "src/renderer/render_data.cpp"


struct Player
{
    Vec2 Position;
    float MoveSpeed; // pixels per second
    Sprite2d Sprite;
};

/*
    NOTE(harsh): allocates using PermanentAllocator
*/
Player* PlayerCreateAndInit(AppMemory* memory)
{
    Player* player = ArenaAlloc<Player>(&memory->PermanentAllocator, sizeof(Player));
    player->Position = {0.0f, 0.0f};
    player->MoveSpeed = 0.10f;
    player->Sprite = {
        {
            MESH_QUAD,
            TEXTURE_ENTITY_ATLAS,
            {96, 48},
        },
        {0, 0},
        {32.0f, 48.0f},
    };

    return player;
}

void PlayerQueueRender(AppMemory* memory, Player* player, RenderData* render_data)
{
    LOG_ASSERT((render_data->commands_count + 1) < render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    RenderCommand render_command = {};
    render_command.MeshId = player->Sprite.SpriteSheet.MeshId;
    render_command.TextureId = player->Sprite.SpriteSheet.TextureId;
    render_command.Instanced = false;

    render_command.Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    // TODO(harsh): implement subpixel rendering instead of this temporary fix of using FloorVec2()
    render_command.Transforms[0] = ModelMat4(FloorVec2(player->Position), player->Sprite.Scale);
    render_command.UvMinMax = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * 1);
    render_command.UvMinMax[0] = GetSpriteUV(&player->Sprite);

    PushRenderCommand(render_data, render_command);
}

/*
    updates the player state and set the render_data.command required for rendering
*/
void PlayerUpdateAndQueueRender(AppMemory* memory, Player* player, InputManager* im, RenderData* render_data)
{
    if (im->IsActionPressed(ACTION_MOVE_UP) || im->IsActionHeld(ACTION_MOVE_UP))
        player->Position.y -= player->MoveSpeed;
    if (im->IsActionPressed(ACTION_MOVE_DOWN) || im->IsActionHeld(ACTION_MOVE_DOWN))
        player->Position.y += player->MoveSpeed;
    if (im->IsActionPressed(ACTION_MOVE_RIGHT) || im->IsActionHeld(ACTION_MOVE_RIGHT))
        player->Position.x += player->MoveSpeed;
    if (im->IsActionPressed(ACTION_MOVE_LEFT) || im->IsActionHeld(ACTION_MOVE_LEFT))
        player->Position.x -= player->MoveSpeed;

    PlayerQueueRender(memory, player, render_data);
}
