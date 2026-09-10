#include <cstdint>

#include "src/win32/renderer/texture_d3d11.h"

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/log.h"

// thirdparty
#include "src/pch.h"


// ====================== Internal functions ======================
namespace
{
    /*
        NOTE(harsh): Expects the textures have sRGB encoded color values, loads them with sRGB format
        hence the values are automatically decoded from sRGB -> Linear

        Creates a texture by loading it from the file at texture_path (allocates using
        temp_allocator, then uploads it to the GPU.

        Returns Texture2D* on success, nullptr otherwise.
    */
    Texture2D* CreateTexture(
        ID3D11Device* device,
        const char* texture_path,
        AppMemory* memory)
    {
        LOG_ASSERT(texture_path, "Invalid texture_path provided");
        LOG_ASSERT(memory, "Invalid memory allocators provided");


        // decode texture file to pixel_data with stbi
        int width;
        int height;
        int channels;
        uint8_t* pixel_data = stbi_load(texture_path, &width, &height, &channels, 0);
        if (!pixel_data)
        {
            LOG_ERROR("Decoding texture file data with stbi failed");
            return nullptr;
        }


        // upload texture to GPU
        D3D11_TEXTURE2D_DESC texture_desc = {};
        texture_desc.Width = width;
        texture_desc.Height = height;
        texture_desc.MipLevels = 1;
        // NOTE(harsh): this has something to do with a texture cube-map, and this value should
        // be in multiple of 6? or me 1 fine cause i'm not uploading a cube texture
        texture_desc.ArraySize = 1;
        // NOTE(harsh): loading even the sRGB texture as non sRGB because the sRGB decoding will happen later
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        // default sampling, no anti-aliasing
        texture_desc.SampleDesc.Count = 1;
        texture_desc.SampleDesc.Quality = 0;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        // Only uploading texture oncea, hence immutable and no CPU access required
        texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
        texture_desc.CPUAccessFlags = 0;
        ID3D11Texture2D* texture_handle = nullptr;
        D3D11_SUBRESOURCE_DATA init_data = {};
        init_data.pSysMem = pixel_data;
        // SysMemPitch is no of bytes each row
        // Calculation: no of pixel * no of bytes per pixel, since each pixel has 3 channel i.e.
        // RGBA, 1 byte each that's 4 bytes per pixel, hence width * 4 = no. of bytes per row
        init_data.SysMemPitch = width * 4;
        init_data.SysMemSlicePitch = 0;
        HRESULT result = device->CreateTexture2D(
            &texture_desc,
            &init_data,
            &texture_handle);
        if (FAILED(result))
        {
            LOG_ERRORF("Failed to create Texture2D with error code: %d", result);
            stbi_image_free(pixel_data);
            return nullptr;
        }
        stbi_image_free(pixel_data);

        // allocate the texture using arena allocator
        Texture2D* texture = (Texture2D*)ArenaAlloc(&memory->PermanentAllocator, sizeof(Texture2D));
        texture->Width = width;
        texture->Height = height;
        texture->Channels = 4;

        // create shader resource view for the texture2d (writes it to the texture->SRV)
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = texture_desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MostDetailedMip = 0;
        srv_desc.Texture2D.MipLevels = 1;
        result = device->CreateShaderResourceView(
            texture_handle,
            &srv_desc,
            &texture->SRV);
        if (FAILED(result))
        {
            LOG_ERRORF("Failed to create Shader Resource View for texture with error code: %d", result);
            texture_handle->Release();
            return nullptr;
        }

        // release the texture handle as its immutable and will never needed to be accessed
        texture_handle->Release();


        return texture;
    }
}; // namespace

/*
    Loads and Uploads all texture to the GPU Memory, All the temporary allocation required
    are done using the temp allocator.
    All the Texture2D allocated on the renderer are allocated using permanent_allocator.
    Texture2D* for the created textures are stored on the texture_array_buffer passed in
*/
int LoadAllTextures(
    AppMemory* memory,
    ID3D11Device* device,
    Texture2D* (&texture_array_buffer)[TEXTURE_COUNT])
{
    const char* texture_paths[TEXTURE_COUNT] = {
        /* TEXTURE_ENTITY_ATLAS */ "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/entity_texture_atlas.png",
        /* TEXTURE_TILEMAP_ATLAS */ "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/tilemap_texture_atlas.png",
    };


    // create and set all texture on the renderer struct
    texture_array_buffer[TEXTURE_ENTITY_ATLAS] = CreateTexture(device, texture_paths[TEXTURE_ENTITY_ATLAS], memory);
    if (!texture_array_buffer[TEXTURE_ENTITY_ATLAS])
        return -1;
    texture_array_buffer[TEXTURE_TILEMAP_ATLAS] = CreateTexture(device, texture_paths[TEXTURE_TILEMAP_ATLAS], memory);
    if (!texture_array_buffer[TEXTURE_TILEMAP_ATLAS])
        return -1;


    return 0;
}
