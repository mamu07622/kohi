#pragma once

#include "core/application.hpp"

// Represents the basic game state in a game. Called for creation by the
// application.
struct Game {
    // The application configuration.
    ApplicationConfiguration ApplicationConfiguration;

    // Function pointer to game's initialise function.
    bool (*Initialise)(Game& game);

    // Function pointer to game's update function.
    bool (*Update)(Game& game, f32 deltaTime);

    // Function pointer to game's render function.
    bool (*Render)(Game& game, f32 deltaTime);

    // Function pointer to handle resizes, if applicable.
    void (*OnResize)(Game& game, u32 width, u32 height);

    // Game-specific game state. Created and managed by game.
    void* State;
};