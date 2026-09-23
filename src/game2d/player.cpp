// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/input/input.h"
#include "src/renderer/render_data.h"

// internal game sub_modules implementations
#include "src/game2d/sprite2d.cpp"


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
    player->Speed = 100.0f;
    player->Sprite = {
        {
            MESH_QUAD,
            TEXTURE_ENTITY_ATLAS,
        },        // sprite sheet
        {0, 0},   // texel coords
        {32, 48}, // scale
        {0, -24}, // sprite offset
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
void PlayerUpdate(Player* player, InputManager* im)
{
}

// Queues the player render command by pushing it to render_data.commands
void PlayerQueueRender(AppMemory* memory, Player* player, double interpolation_alpha, RenderData* render_data)
{
    RenderCommand* render_command = ArenaAlloc<RenderCommand>(&memory->TempAllocator, sizeof(RenderCommand));
    render_command->LayerId = LAYER_FOREGROUND;
    render_command->MeshId = player->Sprite.SpriteSheet.MeshId;
    render_command->TextureId = player->Sprite.SpriteSheet.TextureId;
    render_command->Instanced = false;
    render_command->NoOfInstances = 0;
    render_command->Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    Vec2 interpolated_position = FloorVec(LerpVec(player->PrevPosition, player->Position, interpolation_alpha));
    interpolated_position.x += player->Sprite.Offset.x;
    interpolated_position.y += player->Sprite.Offset.y;
    render_command->Transforms[0] = ModelMat4(
        interpolated_position,
        {(float)player->Sprite.Scale.x, (float)player->Sprite.Scale.y});
    render_command->SortOrder = CalculateSortOrder(player->Position);
    render_command->SpriteCoords = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * 1);
    render_command->SpriteScale = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * 1);
    *render_command->SpriteCoords = player->Sprite.TexelCoords;
    *render_command->SpriteScale = player->Sprite.Scale;

    PushRenderCommand(render_data, render_command);
}
