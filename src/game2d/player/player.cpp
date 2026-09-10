#include "src/game2d/player/player.h"
#include "player.h"
#include "src/utils/arena_allocator.h"
#include "src/utils/globals.h"


Player* PlayerCreateAndInit(AppMemory* memory)
{
    Player* player = (Player*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Player));
    player->Position = {0.0f, 0.0f};
    player->Scale = {32, 48};
    player->MoveSpeed = 120.0f;
    player->Sprite = {
        TEXTURE_ENTITY_ATLAS,
        {0.0f, 0.0f},
    };

    return player;
}

// TODO(harsh): take input manager as input
void PlayerUpdate(AppMemory* memory)
{
}
