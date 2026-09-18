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
    Vec2 PrevPosition;
    Vec2 Position;

    float Speed; // pixels per physics step (there are 100 physics step seconds) so pixel / 100ms
    Sprite2d Sprite;

    // TODO(harsh): add an achor so player is achored on the feet not the center of the body
    // or MAYBE ADD A SPRITE OFFSET THAT WOULD BE SIMPLER
};

/*
    NOTE(harsh): allocates using PermanentAllocator
*/
Player* PlayerCreateAndInit(AppMemory* memory)
{
    Player* player = ArenaAlloc<Player>(&memory->PermanentAllocator, sizeof(Player));
    player->PrevPosition = {0.0f, 0.0f};
    player->Position = {0.0f, 0.0f};
    player->Speed = 300.0f;
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

// runs once per physics tick
void PlayerPhysicsUpdate(AppMemory* memory, double delta_time, Player* player, InputManager* im)
{
    player->PrevPosition = player->Position;

    if (im->IsActionPressed(ACTION_MOVE_UP) || im->IsActionHeld(ACTION_MOVE_UP))
        player->Position.y -= player->Speed * delta_time;
    if (im->IsActionPressed(ACTION_MOVE_DOWN) || im->IsActionHeld(ACTION_MOVE_DOWN))
        player->Position.y += player->Speed * delta_time;
    if (im->IsActionPressed(ACTION_MOVE_RIGHT) || im->IsActionHeld(ACTION_MOVE_RIGHT))
        player->Position.x += player->Speed * delta_time;
    if (im->IsActionPressed(ACTION_MOVE_LEFT) || im->IsActionHeld(ACTION_MOVE_LEFT))
        player->Position.x -= player->Speed * delta_time;
}

// updates the per frame player state
void PlayerUpdate()
{
}

// Queues the player render command by pushing it to render_data.commands
void PlayerQueueRender(AppMemory* memory, Player* player, double interpolation_alpha, RenderData* render_data)
{
    LOG_ASSERT((render_data->commands_count + 1) < render_data->max_commands, "Maximum render commands per frame reached! cannot push more render commands");

    RenderCommand* render_command = ArenaAlloc<RenderCommand>(&memory->TempAllocator, sizeof(RenderCommand));
    render_command->MeshId = player->Sprite.SpriteSheet.MeshId;
    render_command->TextureId = player->Sprite.SpriteSheet.TextureId;
    render_command->Instanced = false;
    render_command->Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    // NOTE(harsh): the position of any entity MUST BE a whole number otherwise the vertex lie in between pixel
    // which gives a distoreded sprite artifact
    Vec2 interpolated_position = FloorVec2(LerpVec2(player->PrevPosition, player->Position, interpolation_alpha));
    render_command->Transforms[0] = ModelMat4(interpolated_position, player->Sprite.Scale);
    render_command->UvMinMax = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * 1);
    render_command->UvMinMax[0] = GetSpriteUV(&player->Sprite);

    PushRenderCommand(render_data, render_command);
}
