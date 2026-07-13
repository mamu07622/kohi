#pragma once

#include "defines.hpp"

struct Game;

// Application configuration.
struct ApplicationConfiguration {
    // Window starting position X axis, if applicable.
    i16 StartPositionX;

    // Window starting position Y axis, if applicable.
    i16 StartPositionY;

    // Window starting width, if applicable.
    i16 StartWidth;

    // Window starting height, if applicable.
    i16 StartHeight;

    // The application name using in windowing, if applicable.
    const char* name;
};

KAPI bool ApplicationCreate(Game& game);

KAPI bool ApplicationRun();