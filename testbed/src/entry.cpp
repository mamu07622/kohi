#include <entry.h>

#include "game.h"

#include <core/kmemory.hpp>

// Define the function to create a game.
bool CreateGame(Game& game) {
    // Application configuration.
    game.ApplicationConfiguration.StartPositionX = 100;
    game.ApplicationConfiguration.StartPositionY = 100;
    game.ApplicationConfiguration.StartWidth = 1280;
    game.ApplicationConfiguration.StartHeight = 720;
    game.ApplicationConfiguration.name = "Kohi Engine Testbed";

    game.Update = GameUpdate;
    game.Render = GameRender;
    game.Initialise = GameInitialise;
    game.OnResize = GameOnResize;

    // Create the game state.
    game.State = KAllocate(sizeof(GameState), MemoryTag::GAME);

    return true;
}