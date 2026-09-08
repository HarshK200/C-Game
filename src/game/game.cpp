#include "src/main.h"
#include "src/game/game.h"
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"

// creates a new game with the "new" keyword and returns the pointer to it
// NOTE(harsh): the allocated memory is not tracker you must track and free the game
// yourself or use an arena allocater, i gotta imlement that allocater first > o <
Game* GameCreateAndInit(AppMemory* memory)
{
    LOG_INFO("Game Init");

    Game* g = (Game*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Game));

    return g;
}

void GameUpdate(Game* g) {}
