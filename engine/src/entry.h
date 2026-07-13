#pragma once

#include "core/application.hpp"
#include "core/kmemory.hpp"
#include "core/logger.hpp"
#include "game-types.hpp"

// Externally-defined function to create a game.
extern bool CreateGame(Game& game);

// The main entry point of the application.
int main(void) {
    InitialiseMemory();

    // Request the game instance from the application.
    Game game {};
    
    if (!CreateGame(game)) {
        KFATAL("Could not create game!");
        return -1;
    }

    // Ensure the function pointers exist.
    if (!game.Render || !game.Update || !game.Initialise || !game.OnResize) {
        KFATAL("The game's function pointers must be assigned!");
        return -2;
    }

    // Initialization.
    if (!ApplicationCreate(game)) {
        KINFO("Application failed to create!.");
        return 1;
    }

    // Begin the game loop.
    if (!ApplicationRun()) {
        KINFO("Application did not shutdown gracefully.");
        return 2;
    }

    ShutdownMemory();

    return 0;
}