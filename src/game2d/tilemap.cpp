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

inline constexpr int TILE_PIXEL_SCALE = 16;
inline constexpr int TILEMAP_SIZE = 20;
inline constexpr int CHUNK_SIZE = 32;
inline constexpr int TILEMAP_CHUNK_HASH_SIZE = 4096;

enum TileType
{
    TILE_GRASS = 0,
    TILE_WATER = 1,
    TILE_DIRT = 2,
    TILE_SAND = 3,
    TILE_TYPE_COUNT,
};

struct Tile
{
    TileType TileType;
    // TODO(harsh): maybe add variant in future? like corrupted or pure
};

struct TileChunk
{
    Vec2i GridCoords;
    Tile Tiles[CHUNK_SIZE * CHUNK_SIZE];

    TileChunk* NextInHash;
};

struct TileMap
{
    // TODO(harsh): what does TILEMAP_CHUNK_BUCKET_SIZE power of 2 affect? is that bad?
    TileChunk* ChunkHash[TILEMAP_CHUNK_HASH_SIZE];

    fnl_state Noise;
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

/*
    Returns the tile anchor's world position *IN PIXELS* that is offset by 0.5 * TILE_PIXEL_SCALE
    in +x and +y direction, because the tile is anchored at the top-left corner to align with the
    tile grid.

    NOTE(harsh): this is so the rendered tiles anchor is the top-left for easier calculations,
    since the calculations are done from from coordinates (0, 0) which without achor tile would be
    centered on which creates a calculation and visual disparity.
    **TLDR: Tile coordinates like (0, 0) is the tile center by default due to rect Quad vertices
    anchor is used to offset that down and right by 0.5 of TILE_PIXEL_SCALE so now (0, 0) is the tile's
    top-left instead of center, which matches calculation**
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

// Returns chunk grid coordinates of the world position passed in.
// World position MUST BE in pixels
Vec2i WorldPosToChunkGridCoords(Vec2 world_pos)
{
    Vec2i result = {};
    result.x = floor(world_pos.x / (CHUNK_SIZE * TILE_PIXEL_SCALE));
    result.y = floor(world_pos.y / (CHUNK_SIZE * TILE_PIXEL_SCALE));
    return result;
}

// Returns the hash of the chunk coordinates passed in
uint32_t HashChunkCoords(Vec2i chunk_coords)
{
    uint32_t x = (uint32_t)chunk_coords.x;
    uint32_t y = (uint32_t)chunk_coords.y;

    // TODO(harsh): why are we using uint32_t? also how in the fuck does this even work?
    uint32_t hash = x * 0x8da6b343;
    hash ^= y * 0xd8163841;

    return hash % TILEMAP_CHUNK_HASH_SIZE;
}

void SetChunkInTilemap(TileMap* tilemap, TileChunk* chunk)
{
    uint32_t chunk_hash = HashChunkCoords(chunk->GridCoords);
    // if no chunk at this hash, set it to chunk passed in
    if (tilemap->ChunkHash[chunk_hash] == nullptr)
    {
        tilemap->ChunkHash[chunk_hash] = chunk;
        return;
    }

    // if there is a chunk at this hash, traverse it until the last node and set it to
    // chunk passed in
    TileChunk* tilemap_chunk = tilemap->ChunkHash[chunk_hash];
    while (tilemap_chunk)
    {
        if (tilemap_chunk->NextInHash)
        {
            tilemap_chunk = tilemap_chunk->NextInHash;
            continue;
        }
        tilemap_chunk->NextInHash = chunk;
        break;
    }
}

/*
    Returns the chunk at chunk_coords in the Tilemap ChunkHash.

    Returns TileChunk* on success and nullptr on failure.
*/
TileChunk* GetChunkInTilemap(TileMap* tilemap, Vec2i chunk_coords)
{
    uint32_t chunk_hash = HashChunkCoords(chunk_coords);
    TileChunk* chunk = tilemap->ChunkHash[chunk_hash];

    while (chunk)
    {
        if (chunk->GridCoords.x == chunk_coords.x &&
            chunk->GridCoords.y == chunk_coords.y)
            return chunk;

        chunk = chunk->NextInHash;
    }

    return nullptr;
}

/*
    For the passed in chunk generates CHUNK_SIZE * CHUNK_SIZE tiles with random tiletype
*/
void GenerateChunkTiles(TileChunk* chunk, fnl_state* noise)
{
    // populating the chunk index in the array
    for (int tile_y = 0; tile_y < CHUNK_SIZE; tile_y++)
    {
        for (int tile_x = 0; tile_x < CHUNK_SIZE; tile_x++)
        {
            Tile* tile = GetTileInChunk(chunk, {tile_x, tile_y});
            Vec2i tile_grid_coords = GetTileGridCoords(chunk->GridCoords, {tile_x, tile_y});

            // sample noise for this tile in tile grid. Value ranges from -1..1
            float noise_sample = fnlGetNoise2D(
                noise,
                tile_grid_coords.x,
                tile_grid_coords.y);

            tile->TileType = TILE_DIRT;
            if (noise_sample < 0.35)
                tile->TileType = TILE_GRASS;
            if (noise_sample < -0.25)
                tile->TileType = TILE_SAND;
            if (noise_sample < -0.30)
                tile->TileType = TILE_WATER;
        }
    }
}

void ChunkQueueRender(AppMemory* memory, TileChunk* chunk, RenderData* render_data)
{
    LOG_ASSERT(chunk, "Invalid chunk passed");

    // push chunk render command
    RenderCommand* render_command = ArenaAlloc<RenderCommand>(&memory->TempAllocator, sizeof(RenderCommand));
    render_command->LayerId = LAYER_TILEMAP;
    render_command->MeshId = MESH_QUAD;
    render_command->TextureId = TEXTURE_TILEMAP_ATLAS;
    render_command->Instanced = true;
    render_command->NoOfInstances = CHUNK_SIZE * CHUNK_SIZE;
    render_command->Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * CHUNK_SIZE * CHUNK_SIZE);
    render_command->SpriteCoords = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * CHUNK_SIZE * CHUNK_SIZE);
    render_command->SpriteScale = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * CHUNK_SIZE * CHUNK_SIZE);

    Vec2 tile_scale = {TILE_PIXEL_SCALE, TILE_PIXEL_SCALE};
    Vec2 tile_pos = {};
    Vec2i tile_coords = {};
    Sprite2d tile_sprite = {
        {
            MESH_QUAD,
            TEXTURE_TILEMAP_ATLAS,
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
            tile_pos = GetTileAnchorPosition(chunk->GridCoords, {tile_x, tile_y});
            render_command->Transforms[tile_idx] = ModelMat4(tile_pos, tile_scale);
            tile_sprite.TexelCoords.x = 0;
            tile_sprite.TexelCoords.y = TILE_PIXEL_SCALE * static_cast<int>(tile->TileType);
            render_command->SpriteCoords[tile_idx] = {0, TILE_PIXEL_SCALE * static_cast<int>(tile->TileType)};
            render_command->SpriteScale[tile_idx] = tile_sprite.Scale;
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


    tilemap->Noise = fnlCreateState();
    tilemap->Noise.seed = 696732902;
    tilemap->Noise.noise_type = FNL_NOISE_PERLIN;
    tilemap->Noise.frequency = 0.01f;
    tilemap->Noise.fractal_type = FNL_FRACTAL_FBM; // how to combine octaves
    tilemap->Noise.octaves = 3;
    tilemap->Noise.lacunarity = 2.0f; // x-axis knob
    tilemap->Noise.gain = 1.0f;       // aka persistance y-axis knob

    // generate a TILEMAP_SIZE x TILEMAP_SIZE chunks tilemap
    for (int chunk_y = -(TILEMAP_SIZE / 2); chunk_y < (TILEMAP_SIZE / 2); chunk_y++)
    {
        for (int chunk_x = -(TILEMAP_SIZE / 2); chunk_x < (TILEMAP_SIZE / 2); chunk_x++)
        {
            TileChunk* chunk = ArenaAlloc<TileChunk>(&memory->PermanentAllocator, sizeof(TileChunk));
            if (!chunk)
            {
                LOG_ASSERT(chunk, "Chunk creation failed");
                continue;
            }
            chunk->GridCoords = {chunk_x, chunk_y};
            GenerateChunkTiles(chunk, &tilemap->Noise);

            SetChunkInTilemap(tilemap, chunk);
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
        Vec2i chunk_coords = player_chunk_coords + surround_chunk_coords[i];
        TileChunk* chunk = GetChunkInTilemap(tilemap, chunk_coords);
        // if chunk doesn't exist continue to next
        if (!chunk)
            continue;

        ChunkQueueRender(memory, chunk, render_data);
    }
}
