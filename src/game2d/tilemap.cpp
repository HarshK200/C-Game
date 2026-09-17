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
inline constexpr int unsigned REGION_SIZE = 16;

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


// =================================================================================
//                                  HELPER FUNCTIONS
// =================================================================================

Vec2i WorldToChunkCoords(Vec2 world_coords)
{
    Vec2 result = {world_coords.x / CHUNK_SIZE * TILE_PIXEL_SCALE, world_coords.y / CHUNK_SIZE * TILE_PIXEL_SCALE};

    return {int(result.x), int(result.x)};
}

Vec2 TileToWorldCoords(Vec2i tile_coords, Vec2i chunk_coords)
{
    Vec2 result = {};
    result.x = (float)((chunk_coords.x * CHUNK_SIZE) + tile_coords.x) * TILE_PIXEL_SCALE;
    result.y = (float)((chunk_coords.y * CHUNK_SIZE) + tile_coords.y) * TILE_PIXEL_SCALE;

    return result;
}


// =================================================================================
//                                TILEMAP GENERATION
// =================================================================================


// TODO(harsh): THIS IS TEMPORARY single tile texture rendered for testing movement, REMOVE THIS
void TEMP_TileMapQueueRender(AppMemory* memory, RenderData* render_data)
{
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
