#pragma once

// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/game2d/sprite2d.cpp"
#include "src/renderer/render_data.cpp"


inline constexpr int unsigned TILE_PIXEL_SCALE = 16;
inline constexpr int unsigned TILEMAP_SIZE = 3;
inline constexpr int unsigned CHUNK_SIZE = 32;

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
    // TODO(harsh): maybe add variant in future? like corrupted or pure
};

struct TileChunk
{
    Vec2i ChunkCoords;
    Tile Tiles[CHUNK_SIZE * CHUNK_SIZE];
};

struct TileMap
{
    TileChunk Chunks[TILEMAP_SIZE * TILEMAP_SIZE];
};


// =================================================================================
//                                  HELPER FUNCTIONS
// =================================================================================

// NOTE(harsh): Tile coordinates are anchored at the top-left corner of the tile grid.
// This returns the world-space center of the tile.
// WARNING(harsh): This world coords MUST NOT be used for changing tile position as these are
// offseted tile_coords
Vec2 TileToWorldCoords(Vec2i chunk_coords, Vec2i tile_coords)
{
    Vec2 result = {};
    result.x = ((chunk_coords.x * CHUNK_SIZE) + tile_coords.x + 0.5f) * TILE_PIXEL_SCALE;
    result.y = ((chunk_coords.y * CHUNK_SIZE) + tile_coords.y + 0.5f) * TILE_PIXEL_SCALE;
    return result;
}

Tile* GetTileInChunk(TileChunk* chunk, Vec2i tile_coords)
{
    return &chunk->Tiles[(CHUNK_SIZE * tile_coords.y) + tile_coords.x];
}
int GetTileIdxInChunk(Vec2i tile_coords)
{
    return (CHUNK_SIZE * tile_coords.y) + tile_coords.x;
}

TileChunk* GetChunkInTilemap(TileMap* tilemap, Vec2i chunk_coords)
{
    return &tilemap->Chunks[(TILEMAP_SIZE * chunk_coords.y) + chunk_coords.x];
}
int GetChunkIdxInTilemap(Vec2i chunk_coords)
{
    return (TILEMAP_SIZE * chunk_coords.y) + chunk_coords.x;
}

/*
    For the passed in chunk generates CHUNK_SIZE * CHUNK_SIZE tiles with random tiletype
    TODO(harsh): use a noise function to  generated determinic chunks
*/
void GenerateChunkTiles(TileChunk* chunk)
{
    for (int tile_y = 0; tile_y < CHUNK_SIZE; tile_y++)
    {
        for (int tile_x = 0; tile_x < CHUNK_SIZE; tile_x++)
        {
            int random_num = std::rand() % TILE_TYPE_COUNT;
            TileType random_tile_type = static_cast<TileType>(random_num);
            Tile* tile = GetTileInChunk(chunk, {tile_x, tile_y});
            tile->TileType = random_tile_type;
        }
    }
}

void ChunkQueueRender(AppMemory* memory, TileChunk* chunk, RenderData* render_data)
{
    // push chunk render command
    RenderCommand* render_command = ArenaAlloc<RenderCommand>(&memory->TempAllocator, sizeof(RenderCommand));
    render_command->MeshId = MESH_QUAD;
    render_command->TextureId = TEXTURE_TILEMAP_ATLAS;
    render_command->Instanced = true;
    render_command->NoOfInstances = CHUNK_SIZE * CHUNK_SIZE;
    render_command->Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * CHUNK_SIZE * CHUNK_SIZE);
    render_command->UvMinMax = ArenaAlloc<Vec4>(&memory->TempAllocator, sizeof(Vec4) * CHUNK_SIZE * CHUNK_SIZE);

    Vec2 tile_scale = {TILE_PIXEL_SCALE, TILE_PIXEL_SCALE};
    Vec2 tile_pos = {};
    Vec2i tile_coords = {};
    Sprite2d tile_sprite = {
        {
            MESH_QUAD,
            TEXTURE_TILEMAP_ATLAS,
            {112, 96},
        },
        {0, 0},
        {TILE_PIXEL_SCALE, TILE_PIXEL_SCALE},
    };


    for (int tile_y = 0; tile_y < CHUNK_SIZE; tile_y++)
    {
        for (int tile_x = 0; tile_x < CHUNK_SIZE; tile_x++)
        {
            int tile_idx = (CHUNK_SIZE * tile_y) + tile_x;
            Tile* tile = GetTileInChunk(chunk, {tile_x, tile_y});
            tile_pos = TileToWorldCoords(chunk->ChunkCoords, {tile_x, tile_y});
            render_command->Transforms[tile_idx] = ModelMat4(tile_pos, tile_scale);
            tile_sprite.TexelCoords.x = 0;
            tile_sprite.TexelCoords.y = TILE_PIXEL_SCALE * static_cast<int>(tile->TileType);
            render_command->UvMinMax[tile_idx] = GetSpriteUV(&tile_sprite);
        }
    }

    PushRenderCommand(render_data, render_command);
}

// =================================================================================
//                              TILEMAP GENERATION
// =================================================================================

TileMap* TileMapCreateAndInit(AppMemory* memory)
{
    // generate a single chunk
    TileMap* tilemap = ArenaAlloc<TileMap>(&memory->PermanentAllocator, sizeof(TileMap));

    for (int chunk_y = 0; chunk_y < TILEMAP_SIZE; chunk_y++)
    {
        for (int chunk_x = 0; chunk_x < TILEMAP_SIZE; chunk_x++)
        {
            TileChunk* chunk = GetChunkInTilemap(tilemap, {chunk_x, chunk_y});
            chunk->ChunkCoords = {chunk_x, chunk_y};
            GenerateChunkTiles(chunk);
        }
    }

    return tilemap;
}

void TileMapQueueRender(AppMemory* memory, TileMap* tilemap, RenderData* render_data)
{
    for (int chunk_y = 0; chunk_y < TILEMAP_SIZE; chunk_y++)
    {
        for (int chunk_x = 0; chunk_x < TILEMAP_SIZE; chunk_x++)
        {
            TileChunk* chunk = GetChunkInTilemap(tilemap, {chunk_x, chunk_y});
            ChunkQueueRender(memory, chunk, render_data);
        }
    }
}
