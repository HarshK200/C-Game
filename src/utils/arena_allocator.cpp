#include <cstdlib>
#include <cstring>

#include "src/main.h"
#include "src/utils/arena_allocator.h"

ArenaAllocator CreateArena(size_t size)
{
    ArenaAllocator arena_allocator = {};
    arena_allocator.memory = (char*)malloc(size);
    if (arena_allocator.memory)
    {
        arena_allocator.capacity = size;
        memset(arena_allocator.memory, 0, size); // set memory to 0
        arena_allocator.used = 0;
    }

    return arena_allocator;
}

/*
    Allocates the memory aligned using the arena passed in, returns a char* to allocated memory on
    success, otherwise returns nullptr on failure.
*/
char* ArenaAlloc(ArenaAllocator* arena_allocator, size_t size)
{
    char* result = nullptr;

    // memory alignment (does something??? TODO(harsh): figure out what this does)
    size_t alignment_size = (size + 7) & ~7;
    if (arena_allocator->used + alignment_size > arena_allocator->capacity)
    {
        PlatformPrintDebug("[ERROR] Arena allocation failed! capacity exceeded");
        return nullptr;
    }

    result = arena_allocator->memory + arena_allocator->used;
    arena_allocator->used += alignment_size;

    return result;
}
