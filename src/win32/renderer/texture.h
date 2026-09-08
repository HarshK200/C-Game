#pragma once

#include "src/win32/renderer/renderer_d3d11.h"

/*
    NOTE(harsh): this has nothing to do with d3d11 render texture or RTV,
    this is just a 2d texture image
*/
struct Texture2D
{
};


HRESULT LoadAllTextures(
    Renderer* r,
    ArenaAllocator* permanent_allocator,
    ArenaAllocator* temp_allocator);
