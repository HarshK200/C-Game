#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "src/utils/log.h"


// ULL here means unsigned long long
#define KiloByte 1024ULL
#define MegaByte ((KiloByte) * 1024ULL)
#define GigaByte ((MegaByte) * 1024ULL)

struct ArenaAllocator
{
    uint8_t* base;
    size_t capacity;
    size_t used;
};

struct AppMemory
{
    ArenaAllocator PermanentAllocator;
    ArenaAllocator TempAllocator;
};


/*
    Creates an arena allocator on the stack, allocates the memory for it using
    malloc(size).
    Returns the ArenaAllocator on success, on failure the returned allocator capacity is 0
    and allocator.base is nullptr
*/
inline ArenaAllocator CreateArena(size_t size)
{
    ArenaAllocator arena_allocator = {};
    arena_allocator.base = (uint8_t*)malloc(size);
    if (!arena_allocator.base)
    {
        LOG_ASSERT(false, "CreateArena malloc failed");
        arena_allocator.base = nullptr;
        arena_allocator.capacity = 0;
        arena_allocator.used = 0;
        return arena_allocator;
    }
    arena_allocator.capacity = size;
    memset(arena_allocator.base, 0, size); // set memory to 0
    arena_allocator.used = 0;

    return arena_allocator;
}

/*
    NOTE(harsh): allocates memory alligned to 16 bytes, expects that malloc() used in CreateArena()
    to return a 16 bytes aligned pointer *Which is does on 64 bit systems*
    *DOES NOT ZERO OUT MEMORY* your allocator should already have the whole memory zeroed out.
    Which it would have if it was reset with ArenaReset or created with CreateArena.

    Allocates the memory of *size_t size* using the arena passed in.
    Returns a char* to allocated memory on success, otherwise returns nullptr on failure.
*/
inline uint8_t* ArenaAlloc(ArenaAllocator* arena_allocator, size_t size)
{
    uint8_t* result = nullptr;

    /*
        memory alignment: rounds size up to next multiple of 16 (clears last 4 bits).
        e.g. size=20 -> 20+15=35 (0010 0011) & ~15 (1111 0000) = 32 (0010 0000)
             size=16 -> 16+15=31 (0001 1111) & ~15 (1111 0000) = 16 (0001 0000) ALREADY ALIGNED HENCE UNCHANGED
        ~15 = (1111 0000), so ANDing always zeroes the last 4 bits -> multiple of 16.
        the +15 is what pushes an unaligned size into the next multiple before truncating.
    */
    size_t alignment_size = (size + 15) & ~15;
    if (alignment_size > arena_allocator->capacity - arena_allocator->used)
    {
        LOG_ASSERT(false, "Arena allocation failed! cannot allocate, arena capacity exceeded");
        return nullptr;
    }

    result = arena_allocator->base + arena_allocator->used;
    arena_allocator->used += alignment_size;

    return result;
}

/*
    NOTE(harsh): *ZERO OUT* the *USED* memory of arena with memset()
    Resets the arena used pointer to 0, hence reseting the arena
*/
inline void ArenaReset(ArenaAllocator* arena_allocator)
{
    memset(arena_allocator->base, 0, arena_allocator->used);
    arena_allocator->used = 0;
}
