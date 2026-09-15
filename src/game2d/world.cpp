#pragma once

// utils
#include "src/game2d/world.cpp"
#include "src/game2d/sprite2d.cpp"
#include "src/renderer/render_data.cpp"
#include "src/utils/arena_allocator.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/log.h"


// TODO(harsh): this is temporary, WORLD_DIMENTIONS should be adjustable by the player
inline constexpr int unsigned WORLD_DIMENTION_IN_CHUNKS = 4; // TEMP 4x4 chunks i.e. 16 chunks in total flat array
inline constexpr int unsigned TILES_PER_CHUNK = 16;
inline constexpr int unsigned TILE_PIXEL_SIZE = 16;

enum TileType
{
    TILE_GRASS = 0,
    TILE_WATER = 1,
    TILE_DIRT = 2,
    TILE_TYPE_COUNT,
};

struct Tile
{
    TileType tile_type;
    Vec2 size;
    Vec2 position;
    Sprite2d sprite;
};

struct Chunk
{
    Tile tiles[TILES_PER_CHUNK][TILES_PER_CHUNK]; // simple flat2d array of tiles

    // TODO(harsh): remove this temp is initialized, this is just for testing
    bool is_initialized = false;
};

// TODO(harsh): this is temporary, polish this
struct World
{
    // TODO(harsh): remove this 2d falt array. Instead replace this with a hashmap for Vec2 chunk_coord -> Chunk chunk
    Chunk chunks[WORLD_DIMENTION_IN_CHUNKS][WORLD_DIMENTION_IN_CHUNKS]; // chunks 4x4
};

// =================================================================================
//                                  HELPER FUNCTIONS
// =================================================================================

Vec2i WorldToChunkCoords(Vec2 world_coords)
{
    Vec2 result = {world_coords.x / TILES_PER_CHUNK * TILE_PIXEL_SIZE, world_coords.y / TILES_PER_CHUNK * TILE_PIXEL_SIZE};

    return {int(result.x), int(result.x)};
}

Vec2 TileToWorldCoords(Vec2i tile_coords, Vec2i chunk_coords)
{
    Vec2 result = {};
    result.x = (float)((chunk_coords.x * TILES_PER_CHUNK) + tile_coords.x) * TILE_PIXEL_SIZE;
    result.y = (float)((chunk_coords.y * TILES_PER_CHUNK) + tile_coords.y) * TILE_PIXEL_SIZE;

    return result;
}


// =================================================================================
//                                    TESTING TEMP
// =================================================================================

/*
    TODO(harsh): this is a temporary function for testing, write a generate world function
    which does procedural world generation and write the world data to disk in a
    32x32 chunks per regions, for O(n) easier reads *learn from minecraft serialization*
*/
World* Temp_GenerateWorld(AppMemory* memory)
{
    World* world = ArenaAlloc<World>(&memory->PermanentAllocator, sizeof(World));

    Chunk* chunk = &world->chunks[0][0];

    for (int tile_y = 0; tile_y < TILES_PER_CHUNK; tile_y++)
    {
        for (int tile_x = 0; tile_x < TILES_PER_CHUNK; tile_x++)
        {
            // TEMP random tile generation
            int random_num = std::rand() % TILE_TYPE_COUNT;
            TileType tile_type = static_cast<TileType>(random_num);

            Tile tile = {};
            tile.position = {(float)TILE_PIXEL_SIZE * tile_x, (float)TILE_PIXEL_SIZE * tile_y};
            tile.tile_type = tile_type;
            tile.size = {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};
            tile.sprite = {
                {
                    MESH_QUAD,
                    TEXTURE_TILEMAP_ATLAS,
                    {112, 96},
                },
                {0, 16 * random_num},
                {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE},
            };

            chunk->tiles[tile_y][tile_x] = tile;
        }
    }
    chunk->is_initialized = true;

    return world;
}


void Temp_TestPushRenderCommand(AppMemory* memory, RenderData* render_data, World* world)
{
    Chunk* chunk = &world->chunks[0][0];
    RenderCommand render_command = {};
    render_command.mesh_id = MESH_QUAD;
    render_command.texture_id = TEXTURE_TILEMAP_ATLAS;
    render_command.instanced = true;
    render_command.no_of_instances = 0;

    render_command.transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * TILES_PER_CHUNK * TILES_PER_CHUNK);
    render_command.uv_min_max = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * TILES_PER_CHUNK * TILES_PER_CHUNK);

    for (int tile_y = 0; tile_y < TILES_PER_CHUNK; tile_y++)
    {
        for (int tile_x = 0; tile_x < TILES_PER_CHUNK; tile_x++)
        {
            Tile* tile = &chunk->tiles[tile_y][tile_x];
            render_command.transforms[render_command.no_of_instances] = ModelMat4(
                tile->position,
                {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE});
            render_command.uv_min_max[render_command.no_of_instances] = GetSpriteUV(
                &tile->sprite);
            render_command.no_of_instances += 1;
        }
    }

    PushRenderCommand(render_data, render_command);
}
