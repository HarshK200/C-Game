// utils
#include "src/utils/log.h"
#include "src/utils/arena_allocator.h"

// internal game sub_modules implementations
#include "src/game2d/sprite2d.cpp" // sprite2d is a shadered module
#include "src/game2d/player.cpp"
#include "src/game2d/tilemap.cpp"
#include "src/game2d/camera2d.cpp"


// ================== Renderer Layer Services Definitions ==================
struct Game
{
    Camera2d* Camera;
    Player* Player;
    TileMap* TileMap;
};

// creates a new game with the "new" keyword and returns the pointer to it
// NOTE(harsh): the allocated memory is not tracker you must track and free the game
// yourself or use an arena allocater, i gotta imlement that allocater first > o <
Game* GameCreateAndInit(AppMemory* memory)
{
    LOG_INFO("Game Init");

    Game* g = ArenaAlloc<Game>(&memory->PermanentAllocator, sizeof(Game));

    g->Camera = Camera2dCreateAndInit(memory);
    g->Player = PlayerCreateAndInit(memory);
    g->TileMap = TileMapCreateAndInit(memory);

    return g;
}


// This runs once per physics timestep completely seperate from frame update
void GamePhysicsUpdate(AppMemory* memory, double delta_time, Game* g, InputManager* input_manager)
{
    Camera2dPhysicsUpdate(g->Camera, delta_time, g->Player->Position);
    PlayerPhysicsUpdate(memory, delta_time, g->Player, input_manager);
}

// Updates the Per Frame game state
void GameUpdate(AppMemory* memory, Game* g, InputManager* input_manager)
{
    Camera2dUpdate();
    PlayerUpdate();
}

void GameQueueRender(AppMemory* memory, Game* g, RenderData* render_data, double interpolation_alpha)
{
    Camera2dQueueRender(g->Camera, interpolation_alpha, render_data);

    TileMapQueueRender(memory, g->TileMap, g->Player->Position, render_data);
    PlayerQueueRender(memory, g->Player, interpolation_alpha, render_data);
}
