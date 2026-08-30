#include "main.h"


struct Game
{
};


Game* GameNew()
{
    PlatformPrintDebug("Game Init\n");
    // TODO(harsh): allocate using arena allocator
    Game* g = new Game{};

    return g;
}

void GameUpdate(Game* g) {}
