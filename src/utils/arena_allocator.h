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
    Creates an arena allocator on the stack and returns it
*/
inline ArenaAllocator CreateArena(size_t size)
{
    ArenaAllocator arena_allocator = {};
    arena_allocator.base = malloc(size);
    if (!arena_allocator.base)
    {
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

    // memory alignment (does something??? TODO(harsh): figure out what this does)
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
