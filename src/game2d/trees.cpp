// utils
#include "src/utils/log.h"
#include "src/utils/enums.h"
#include "src/utils/game_math.h"
#include "src/utils/arena_allocator.h"

// layers glue
#include "src/renderer/render_data.h"

// internal game sub_modules implementations
#include "src/game2d/sprite2d.cpp"

struct Tree
{
    Vec2 Position;
    Sprite2d Sprite;
};

Tree* TreesCreateAndInit(AppMemory* memory)
{
    Tree* tree = ArenaAlloc<Tree>(&memory->PermanentAllocator, sizeof(Tree) * 1);
    tree->Position = {50, 0};
    tree->Sprite = {
        {
            MESH_QUAD,
            TEXTURE_ENTITY_ATLAS,
        },        // sprite sheet
        {0, 48},  // texel coords
        {80, 96}, // size
        {0, -48}, // sprite offset
    };

    return tree;
}

void TreesQueueRender(AppMemory* memory, Tree* tree, RenderData* render_data)
{
    RenderCommand* render_command = ArenaAlloc<RenderCommand>(&memory->TempAllocator, sizeof(RenderCommand));
    render_command->LayerId = LAYER_FOREGROUND;
    render_command->MeshId = tree->Sprite.SpriteSheet.MeshId;
    render_command->TextureId = tree->Sprite.SpriteSheet.TextureId;
    // TODO(harsh): this is just a temp test, make this an instanced draw in the future
    render_command->Instanced = false;
    render_command->NoOfInstances = 0;
    render_command->Transforms = ArenaAlloc<Mat4>(&memory->TempAllocator, sizeof(Mat4) * 1);
    render_command->Transforms[0] = ModelMat4(
        tree->Position + tree->Sprite.Offset,
        {(float)tree->Sprite.Scale.x, (float)tree->Sprite.Scale.y});
    render_command->SortOrder = CalculateSortOrder(tree->Position);
    render_command->SpriteCoords = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * 1);
    render_command->SpriteScale = ArenaAlloc<Vec2i>(&memory->TempAllocator, sizeof(Vec2i) * 1);
    *render_command->SpriteCoords = tree->Sprite.TexelCoords;
    *render_command->SpriteScale = tree->Sprite.Scale;

    PushRenderCommand(render_data, render_command);
}
