#pragma once

// utils
#include "src/utils/game_math.h"

#define WORLD_DIM 64;

struct Chunk
{
};

struct World
{
    /*
        TODO(harsh): for the sake of renderer sorting and testing and implementing input
        just put a flat 2d array for chunks here and call it a day for now.

        Later revisit this when the input->action_map and layered renderering + instance
        rendering is done.
        use hashmap + preload a 32x32 chunk region into memory not the whole world chunks
        Chunk data can be serialized into memory similar to how minecraft does it. *look at up*
    */
    Chunk** chunks; // this is a flat 2d array [chunk_coord_x][chunk_coord_y]
};

World* GenerateWorld()
{
    return nullptr;
}
