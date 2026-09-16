#pragma once

#include "src/main.h"

// utils
#include "src/utils/log.h"
#include "src/utils/arena_allocator.h"

#include "src/game2d/player.cpp"
#include "src/game2d/camera2d.cpp"
#include "src/game2d/tilemap.cpp"


// ================== Renderer Layer Services Definitions ==================
struct Game2d
{
    Camera2d* Camera;
    Player* Player;
    TileMap* Tilemap;
};

// creates a new game with the "new" keyword and returns the pointer to it
// NOTE(harsh): the allocated memory is not tracker you must track and free the game
// yourself or use an arena allocater, i gotta imlement that allocater first > o <
Game2d* GameCreateAndInit(AppMemory* memory)
{
    LOG_INFO("Game Init");

    Game2d* g = ArenaAlloc<Game2d>(&memory->PermanentAllocator, sizeof(Game2d));

    g->Camera = Camera2dCreateAndInit(memory);
    g->Player = PlayerCreateAndInit(memory);

    return g;
}

/*
    Updates the game state and write the required information to render a frame to
    RenderData* passed in
*/
void GameUpdate(AppMemory* memory, Game2d* g, InputManager* input_manager, RenderData* render_data)
{
    Camera2dUpdate(g->Camera, render_data);
    TileMapUpdateAndQueueRender(memory, render_data);
    PlayerUpdateAndQueueRender(memory, g->Player, input_manager, render_data);
}
