#pragma once

// utils
#include "src/game2d/sprite2d.cpp"
#include "src/utils/game_math.h"


// TODO(harsh): this is temporary, WORLD_DIMENTIONS should be adjustable by the player
inline constexpr int unsigned WORLD_DIMENTIONS = 8;
inline constexpr int unsigned CHUNK_SIZE = 16;

enum TileType
{
    TILE_GRASS = 0,
    TILE_WATER = 1,
    TILE_DIRT = 1,
    TILE_TYPE_COUNT,
};

struct Tile
{
    TileType tile_type;
    Vec2 position;
    Vec2 scale;
    Sprite2d sprite;
};

struct Chunk
{
    Tile tiles[CHUNK_SIZE][CHUNK_SIZE];
};

// TODO(harsh): this is temporary, polish this
struct World
{
    // TODO(harsh): need a hashmap for Vec2 chunk_coord -> Chunk chunk
};


/*
    TODO(harsh): this is a temporary function for testing, write a generate world function
    which does procedural world generation and write the world data to disk in a
    32x32 chunk regions, for O(n) easier reads
*/
World* GenerateWorldAndPushRender(AppMemory* memory)
{
    World* world = ArenaAlloc<World>(&memory->TempAllocator, sizeof(World));

    // loop through each chunk

    return world;
}
