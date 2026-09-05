#pragma once
#include <cstddef>


struct ArenaAllocator
{
    char* memory;
    size_t capacity;
    size_t used;
};

ArenaAllocator CreateArena(size_t size);
