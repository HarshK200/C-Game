// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

#include "src/renderer/render_data.h"
#include "src/game2d/sprite2d.cpp"

// thirdparty
#ifndef FNL_IMPL
#define FNL_IMPL
#include "third_party/fast_noise_lite.h"
#endif

inline constexpr int unsigned TILE_PIXEL_SCALE = 16;
inline constexpr int unsigned TILEMAP_SIZE = 5;
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
    // TODO(harsh): implement a hash table for large sparse data and region data loading
    TileChunk Chunks[TILEMAP_SIZE * TILEMAP_SIZE];
    int Seed;
    float NoiseScale;
};


// =================================================================================
//                                  HELPER FUNCTIONS
// =================================================================================

/*
    Returns tile's world tile coordinates in the tile grid
*/
Vec2i GetTileGridCoords(Vec2i chunk_coords, Vec2i tile_local_coords)
{
    Vec2i result = {};
    result.x = (chunk_coords.x * CHUNK_SIZE) + tile_local_coords.x;
    result.y = (chunk_coords.y * CHUNK_SIZE) + tile_local_coords.y;
    return result;
}

/*
    Returns the tile world position *IN PIXELS*.
*/
Vec2 GetTileWorldPosition(Vec2i chunk_coords, Vec2i tile_local_coords)
{
    Vec2 result = {};
    result.x = ((chunk_coords.x * CHUNK_SIZE) + tile_local_coords.x) * TILE_PIXEL_SCALE;
    result.y = ((chunk_coords.y * CHUNK_SIZE) + tile_local_coords.y) * TILE_PIXEL_SCALE;
    return result;
}

// Returns chunk grid coordinates of the world position passed in.
// World position MUST BE in pixels
Vec2i WorldPosToChunkGridCoords(Vec2 world_pos)
{
    Vec2i result = {};
    result.x = (int)(world_pos.x / (CHUNK_SIZE * TILE_PIXEL_SCALE));
    result.y = (int)(world_pos.y / (CHUNK_SIZE * TILE_PIXEL_SCALE));
    return result;
}

/*
    Returns the tile anchor's world position *IN PIXELS* that is offset by 0.5 * TILE_PIXEL_SCALE
    in +x and +y direction, because the tile is anchored at the top-left corner to align with the
    tile grid.

    NOTE(harsh): this is so the rendered tiles match the calculations, as calculations are
    not done based on anchor, they are based
*/
Vec2 GetTileAnchorPosition(Vec2i chunk_coords, Vec2i tile_local_coords)
{
    Vec2 result = {};
    result.x = ((chunk_coords.x * CHUNK_SIZE) + tile_local_coords.x + 0.5f) * TILE_PIXEL_SCALE;
    result.y = ((chunk_coords.y * CHUNK_SIZE) + tile_local_coords.y + 0.5f) * TILE_PIXEL_SCALE;
    return result;
}

Tile* GetTileInChunk(TileChunk* chunk, Vec2i tile_local_coords)
{
    return &chunk->Tiles[(CHUNK_SIZE * tile_local_coords.y) + tile_local_coords.x];
}
int GetTileIdxInChunk(Vec2i tile_local_coords)
{
    return (CHUNK_SIZE * tile_local_coords.y) + tile_local_coords.x;
}

TileChunk* GetChunkInTilemap(TileMap* tilemap, Vec2i chunk_coords)
{
    // return null on invalid chunk
    if (chunk_coords.x >= TILEMAP_SIZE || chunk_coords.y >= TILEMAP_SIZE)
        return nullptr;

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
void GenerateChunkTiles(TileChunk* chunk, fnl_state* noise, float noise_scale)
{
    // populating the chunk index in the array
    for (int tile_y = 0; tile_y < CHUNK_SIZE; tile_y++)
    {
        for (int tile_x = 0; tile_x < CHUNK_SIZE; tile_x++)
        {
            Tile* tile = GetTileInChunk(chunk, {tile_x, tile_y});
            Vec2i tile_grid_coords = GetTileGridCoords(chunk->ChunkCoords, {tile_x, tile_y});

            // sample noise for this tile in tile grid. Value ranges from 0..1
            // TODO(harsh): multiply the coordinates by somearbitary scale for more control?
            float noise_sample = fnlGetNoise2D(
                noise,
                tile_grid_coords.x * noise_scale,
                tile_grid_coords.y * noise_scale);

            tile->TileType = TILE_WATER;
            if (noise_sample > 0.05)
                tile->TileType = TILE_GRASS;
            if (noise_sample > 0.15)
                tile->TileType = TILE_DIRT;
        }
    }
}

void ChunkQueueRender(AppMemory* memory, TileChunk* chunk, RenderData* render_data)
{
    LOG_ASSERT(chunk, "Invalid chunk passed");

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
            tile_pos = GetTileAnchorPosition(chunk->ChunkCoords, {tile_x, tile_y});
            render_command->Transforms[tile_idx] = ModelMat4(tile_pos, tile_scale);
            tile_sprite.TexelCoords.x = 0;
            tile_sprite.TexelCoords.y = TILE_PIXEL_SCALE * static_cast<int>(tile->TileType);
            render_command->UvMinMax[tile_idx] = GetSpriteUV(&tile_sprite);
        }
    }

    PushRenderCommand(render_data, render_command);
}


// =================================================================================
//                              EXPORTED FUNCTIONS
// =================================================================================

TileMap* TileMapCreateAndInit(AppMemory* memory)
{
    TileMap* tilemap = ArenaAlloc<TileMap>(&memory->PermanentAllocator, sizeof(TileMap));
    tilemap->Seed = 696732902;

    /*
        create noise TODO(harsh): maybe put the noise on the tilemap? i donno if its needed
        or not though.
    */
    fnl_state noise = fnlCreateState();
    noise.seed = tilemap->Seed;
    // TODO(harsh): experiment with OpenSimplexNoise as well
    noise.noise_type = FNL_NOISE_PERLIN;
    noise.frequency = 0.01;
    tilemap->NoiseScale = 10.0;

    // generate a TILEMAP_SIZE x TILEMAP_SIZE chunks tilemap
    // TODO(harsh): use hashtable based chunk generation
    for (int chunk_y = 0; chunk_y < TILEMAP_SIZE; chunk_y++)
    {
        for (int chunk_x = 0; chunk_x < TILEMAP_SIZE; chunk_x++)
        {
            TileChunk* chunk = GetChunkInTilemap(tilemap, {chunk_x, chunk_y});
            if (!chunk)
                continue;
            chunk->ChunkCoords = {chunk_x, chunk_y};

            GenerateChunkTiles(chunk, &noise, tilemap->NoiseScale);
        }
    }

    return tilemap;
}

void TileMapQueueRender(
    AppMemory* memory,
    TileMap* tilemap,
    Vec2 player_position,
    RenderData* render_data)
{
    // only render the chunks in a 1 chunk radius around the player
    Vec2i surround_chunk_coords[9] = {
        {-1, 0},
        {0, 0},
        {1, 0},
        {-1, -1},
        {0, -1},
        {1, -1},
        {-1, 1},
        {0, 1},
        {1, 1},
    };

    Vec2i player_chunk_coords = WorldPosToChunkGridCoords(player_position);
    for (int i = 0; i < 9; i++)
    {
        Vec2i chunk_coords = player_chunk_coords;
        if (((chunk_coords.x + surround_chunk_coords[i].x) < 0) ||
            ((chunk_coords.y + surround_chunk_coords[i].y) < 0))
            continue;

        chunk_coords.x += surround_chunk_coords[i].x;
        chunk_coords.y += surround_chunk_coords[i].y;

        TileChunk* chunk = GetChunkInTilemap(tilemap, chunk_coords);
        if (!chunk)
            continue;

        ChunkQueueRender(memory, chunk, render_data);
    }
}
