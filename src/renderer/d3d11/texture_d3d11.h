#pragma once

#include <d3d11.h>

// utils
#include "src/utils/enums.h"
#include "src/utils/arena_allocator.h"

struct Texture2D
{
    int unsigned Width;
    int unsigned Height;
    int unsigned Channels;
    ID3D11ShaderResourceView* SRV;
};


int LoadAllTextures(
    AppMemory* memory,
    ID3D11Device* device,
    Texture2D* (&texture_array_buffer)[TEXTURE_COUNT]);
