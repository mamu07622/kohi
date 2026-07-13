#pragma once

#include "core/application.hpp"

// Represents the basic game state in a game. Called for creation by the
// application.
struct Game {
    // The application configuration.
    ApplicationConfiguration applicationConfiguration;

    // Function pointer to game's initialise function.
    bool (*initialise)(Game& game);

    // Function pointer to game's update function.
    bool (*update)(Game& game, f32 deltaTime);

    // Function pointer to game's render function.
    bool (*render)(Game& game, f32 deltaTime);

    // Function pointer to handle resizes, if applicable.
    void (*onResize)(Game& game, u32 width, u32 height);

    // Game-specific game state. Created and managed by game.
    void* state;
};