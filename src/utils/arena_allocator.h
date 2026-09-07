#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "src/main.h"
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"


struct ArenaAllocator
{
    void* base;
    size_t capacity;
    size_t used;
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
    arena_allocator.base = malloc(size);
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
    Allocates the memory aligned using the arena passed in, returns a char* to allocated memory on
    success, otherwise returns nullptr on failure.
*/
inline char* ArenaAlloc(ArenaAllocator* arena_allocator, size_t size)
{
    char* result = nullptr;

    /*
        memory alignment: rounds size up to next multiple of 8 (clears last 3 bits).
        e.g. size=10 -> 10+7=17 (00010001) & ~7 (11111000) = 16 (00010000)
             size=16 -> 16+7=23 (00010111) & ~7            = 16 (already aligned, unchanged)
        ~7 = ...11111000, so ANDing always zeroes the last 3 bits -> multiple of 8.
        the +7 is what pushes an unaligned size into the next multiple before truncating.
    */
    size_t alignment_size = (size + 7) & ~7;
    if (arena_allocator->used + alignment_size > arena_allocator->capacity)
    {
        LOG_ASSERT(false, "Arena allocation failed! capacity exceeded");
        return nullptr;
    }

    result = (char*)arena_allocator->base + arena_allocator->used;
    arena_allocator->used += alignment_size;

    return result;
}
