#pragma once

#include "src/win32/renderer/renderer_d3d11.h"

/*
    NOTE(harsh): this has nothing to do with d3d11 render texture or RTV,
    this is just a 2d texture image
*/
struct Texture2D
{
};


void LoadAllTextures(Renderer* r, ArenaAllocator* temp_allocator);
