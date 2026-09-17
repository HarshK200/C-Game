#pragma once

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/game2d/sprite2d.cpp"
#include "src/renderer/render_data.cpp"


inline constexpr int unsigned TILE_PIXEL_SCALE = 16;

// TODO(harsh): Make teh CHUNKS_PER_TILEMAP adjustable as world size from world gen screen
// TEMP 16x16 chunks per TileMap which is just the world
inline constexpr int unsigned CHUNKS_PER_TILEMAP = 16;
inline constexpr int unsigned TILES_PER_CHUNK = 16;

enum TileType
{
    TILE_GRASS = 0,
    TILE_WATER = 1,
    TILE_DIRT = 2,
    TILE_TYPE_COUNT,
};

struct Tile
{
    TileType TileType;
    Vec2 Position;
    Sprite2d Sprite;
};

struct TileChunk
{
    Vec2 ChunkCoords;
    Tile* Tiles[TILES_PER_CHUNK]; // 1d Tile array pointer

    TileChunk* NextInHash;
};


struct TileMap
{
    TileChunk* TileChunks[CHUNKS_PER_TILEMAP];
};


// =================================================================================
//                                  HELPER FUNCTIONS
// =================================================================================

Vec2i WorldToChunkCoords(Vec2 world_coords)
{
    Vec2 result = {world_coords.x / TILES_PER_CHUNK * TILE_PIXEL_SCALE, world_coords.y / TILES_PER_CHUNK * TILE_PIXEL_SCALE};

    return {int(result.x), int(result.x)};
}

Vec2 TileToWorldCoords(Vec2i tile_coords, Vec2i chunk_coords)
{
    Vec2 result = {};
    result.x = (float)((chunk_coords.x * TILES_PER_CHUNK) + tile_coords.x) * TILE_PIXEL_SCALE;
    result.y = (float)((chunk_coords.y * TILES_PER_CHUNK) + tile_coords.y) * TILE_PIXEL_SCALE;

    return result;
}

// TODO(harsh): watch handmade hero and implement tilemap system based on that
TileChunk* GetTileChunk(TileMap* tilemap)
{
    return nullptr;
}

// =================================================================================
//                                  TILEMAP GENERATION
// =================================================================================
TileMap* TileMapInit()
{
    return nullptr;
}

void TEMP_TileMapQueueRender(AppMemory* memory, RenderData* render_data)
{
    // TODO(harsh): THIS IS TEMPORARY single tile rendered for testing movement, REMOVE THIS
    RenderCommand render_command = {};
    render_command.MeshId = MESH_QUAD;
    render_command.TextureId = TEXTURE_TILEMAP_ATLAS;
    render_command.Instanced = false;
    render_command.NoOfInstances = 0;
    render_command.Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    Vec2 position = {10.0f, 5.0f};
    Vec2 scale = {TILE_PIXEL_SCALE, TILE_PIXEL_SCALE};
    render_command.Transforms[0] = ModelMat4(position, scale);
    Sprite2d sprite = {
        {
            MESH_QUAD,
            TEXTURE_TILEMAP_ATLAS,
            {112, 96},
        },
        {0, 16},
        {TILE_PIXEL_SCALE, TILE_PIXEL_SCALE},
    };
    render_command.UvMinMax = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * 1);
    render_command.UvMinMax[0] = GetSpriteUV(&sprite);
    PushRenderCommand(render_data, render_command);
}


// =================================================================================
//                                  TESTING TEMP
// =================================================================================
/*
    TODO(harsh): this is a temporary function for testing, write a generate world function
    which does procedural world generation and write the world data to disk in a
    32x32 chunks per regions, for O(n) easier reads *learn from minecraft serialization*
*/
// TileMap* Temp_GenerateTileMap(AppMemory* memory)
// {
//     TileMap* world = ArenaAlloc<TileMap>(&memory->PermanentAllocator, sizeof(TileMap));
//
//     TileChunk* chunk = &world->chunks[0][0];
//
//     for (int tile_y = 0; tile_y < TILES_PER_CHUNK; tile_y++)
//     {
//         for (int tile_x = 0; tile_x < TILES_PER_CHUNK; tile_x++)
//         {
//             // TEMP random tile generation
//             int random_num = std::rand() % TILE_TYPE_COUNT;
//             TileType tile_type = static_cast<TileType>(random_num);
//
//             Tile tile = {};
//             tile.position = {(float)TILE_PIXEL_SIZE * tile_x, (float)TILE_PIXEL_SIZE * tile_y};
//             tile.tile_type = tile_type;
//             tile.size = {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};
//             tile.sprite = {
//                 {
//                     MESH_QUAD,
//                     TEXTURE_TILEMAP_ATLAS,
//                     {112, 96},
//                 },
//                 {0, 16 * random_num},
//                 {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE},
//             };
//
//             chunk->tiles[tile_y][tile_x] = tile;
//         }
//     }
//
//     return world;
// }
// void Temp_PushTileMapRenderCommands(AppMemory* memory, RenderData* render_data, TileMap* tilemap)
// {
//     // TODO(harsh): remove this
//     // Only rendering one chunk for TESTING
//     TileChunk* chunk = &tilemap->chunks[0][0];
//
//     RenderCommand render_command = {};
//     render_command.mesh_id = MESH_QUAD;
//     render_command.texture_id = TEXTURE_TILEMAP_ATLAS;
//     render_command.instanced = true;
//     render_command.no_of_instances = 0;
//
//     render_command.transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * TILES_PER_CHUNK * TILES_PER_CHUNK);
//     render_command.uv_min_max = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * TILES_PER_CHUNK * TILES_PER_CHUNK);
//
//     for (int tile_y = 0; tile_y < TILES_PER_CHUNK; tile_y++)
//     {
//         for (int tile_x = 0; tile_x < TILES_PER_CHUNK; tile_x++)
//         {
//             Tile* tile = &chunk->tiles[tile_y][tile_x];
//             render_command.transforms[render_command.no_of_instances] = ModelMat4(
//                 tile->position,
//                 {TILE_PIXEL_SIZE, TILE_PIXEL_SIZE});
//             render_command.uv_min_max[render_command.no_of_instances] = GetSpriteUV(
//                 &tile->sprite);
//             render_command.no_of_instances += 1;
//         }
//     }
//
//     PushRenderCommand(render_data, render_command);
// }
