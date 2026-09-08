#include "texture.h"

// utils
#include "src/utils/arena_allocator.h"
#include "src/utils/file_io.h"
#include "src/utils/log.h"

// thirdparty
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"


// ====================== Internal functions ======================
namespace
{
    /*
        Creates a texture by loading it from the file at texture_path (allocates using
        temp_allocator, then uploads it to the GPU.

        Returns Texture2D* on success, nullptr otherwise.
    */
    Texture2D* CreateTexture(
        const char* texture_path,
        ArenaAllocator* permanent_allocator,
        ArenaAllocator* temp_allocator)
    {
        LOG_ASSERT(texture_path, "Invalid texture_path provided");
        LOG_ASSERT(permanent_allocator, "Invalid permanent_allocator provided");
        LOG_ASSERT(temp_allocator, "Invalid temp_allocator provided");

        // load texture file
        int texture_filesize;
        char* file_buffer = GameFileIO::ReadFile(
            temp_allocator,
            texture_path,
            &texture_filesize);
        if (!file_buffer)
        {
            LOG_ERRORF("Unable to load texture file at path: %s", texture_path);
            return nullptr;
        }

        // use stb to decode texture file

        return nullptr;
    }
}; // namespace

/*
    Loads and Uploads all texture to the GPU Memory, All the temporary allocation required
    are done using the temp allocator.
    All the Texture2D allocated on the renderer are allocated using permanent_allocator.
    Textures are stored on the renderer Textures array.
*/
HRESULT LoadAllTextures(
    Renderer* r,
    ArenaAllocator* permanent_allocator,
    ArenaAllocator* temp_allocator)
{
    const char* texture_paths[] = {
        "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/entity_texture_atlas.png",
        "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/tilemap_texture_atlas.png",
    };


    // TODO(harsh): loop through all the texture_paths and call CreateTexture for them


    return S_OK;
}
