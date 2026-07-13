#pragma once

#include "defines.hpp"

struct Game;

// Application configuration.
struct ApplicationConfiguration {
    // Window starting position X axis, if applicable.
    i16 startPositionX;

    // Window starting position Y axis, if applicable.
    i16 startPositionY;

    // Window starting width, if applicable.
    i16 startWidth;

    // Window starting height, if applicable.
    i16 startHeight;

    // The application name using in windowing, if applicable.
    const char* name;
};

KAPI bool ApplicationCreate(Game& game);

KAPI bool ApplicationRun();