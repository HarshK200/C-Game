#include "src/main.h"

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"

#include "src/game2d/camera2d.h"
#include "src/game2d/player.h"


// ================== Renderer Layer Services Definitions ==================
struct Game2d
{
    Camera2d* camera;
    Player* player;
};

// creates a new game with the "new" keyword and returns the pointer to it
// NOTE(harsh): the allocated memory is not tracker you must track and free the game
// yourself or use an arena allocater, i gotta imlement that allocater first > o <
Game2d* GameCreateAndInit(AppMemory* memory)
{
    LOG_INFO("Game Init");

    Game2d* g = (Game2d*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Game2d));

    g->camera = Camera2dCreateAndInit(memory);
    g->player = PlayerCreateAndInit(memory);

    return g;
}

/*
    Updates the game state and write the required information to render a frame to
    RenderData* passed in
*/
void GameUpdate(Game2d* g, RenderData* render_data)
{
    Camera2dUpdate(g->camera, render_data);
    PlayerUpdate(g->player, render_data);
}
