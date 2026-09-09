#pragma once

#include <d3d11.h>

#include "src/main.h"
#include "src/utils/arena_allocator.h"

enum TextureID
{
    TEXTURE_ENTITY_ATLAS = 0,
    TEXTURE_TILEMAP_ATLAS = 1,
    TEXTURE_COUNT
};

struct Texture2D
{
    int unsigned Width;
    int unsigned Height;
    int unsigned Channels;
    ID3D11ShaderResourceView* SRV;
};


int LoadAllTextures(
    Renderer* r,
    AppMemory* memory);
