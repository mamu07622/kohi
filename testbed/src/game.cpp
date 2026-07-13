#include "game.h"

#include <core/logger.hpp>

bool GameInitialise([[maybe_unused]] Game& game) {
    KDEBUG("GameInitialised() called!");

    return true;
}

bool GameUpdate([[maybe_unused]] Game& game, [[maybe_unused]] f32 deltaTime) {
    return true;
}

bool GameRender([[maybe_unused]] Game& game, [[maybe_unused]] f32 deltaTime) {
    return true;
}

void GameOnResize([[maybe_unused]] Game& game, [[maybe_unused]] u32 width,
                  [[maybe_unused]] u32 height) {
    return;
}