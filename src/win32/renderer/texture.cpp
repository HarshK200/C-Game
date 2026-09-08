#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"


/*
    Loads all texture temporarly allocates memory via the temp allocator, the allocated
    textures are stored the renderer Textures array
*/
void LoadAllTextures(Renderer* r, ArenaAllocator* temp_allocator)
{
    const char* texture_paths[] = {
        "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/entity_texture_atlas.png",
        "C:/Users/Harsh/Desktop/personal_dev/cpp_game/assets/textures/tilemap_texture_atlas.png",
    };
}
