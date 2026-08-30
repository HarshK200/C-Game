#include "main.h"


struct Game
{
};


// creates a new game with the "new" keyword and returns the pointer to it
// NOTE(harsh): the allocated memory is not tracker you must track and free the game
// yourself or use an arena allocater, i gotta imlement that allocater first > o <
Game* GameCreateAndInit()
{
    PlatformPrintDebug("Game Init\n");
    // TODO(harsh): allocate using arena allocator
    Game* g = new Game{};

    return g;
}

void GameUpdate(Game* g) {}
