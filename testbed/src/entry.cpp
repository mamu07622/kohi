#include "game.h"

#include <entry.h>

// TODO: Remove this
#include <platform/platform.hpp>

// Define the function to create a game.
bool CreateGame(Game& game) {
    // Application configuration.
    game.applicationConfiguration.startPositionX = 100;
    game.applicationConfiguration.startPositionY = 100;
    game.applicationConfiguration.startWidth = 1280;
    game.applicationConfiguration.startHeight = 720;
    game.applicationConfiguration.name = "Kohi Engine Testbed";

    game.update = GameUpdate;
    game.render = GameRender;
    game.initialise = GameInitialise;
    game.onResize = GameOnResize;

    // Create the game state.
    game.state = PlatformAllocate(sizeof(GameState), false);

    return true;
}