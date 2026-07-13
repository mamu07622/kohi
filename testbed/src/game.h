#pragma once

#include <defines.hpp>
#include <game-types.hpp>

struct GameState {
    f32 deltaTime;
};

bool GameInitialise(Game& game);

bool GameUpdate(Game& game, f32 deltaTime);

bool GameRender(Game& game, f32 deltaTime);

void GameOnResize(Game& game, u32 width, u32 height);