#pragma once
#include "game_types.h"

namespace tetris {
namespace render {

void show_main_menu();
void show_game(const GameState& game, bool help = false);
void show_two_player_game(const GameState& first, const GameState& second);

} // namespace render
} // namespace tetris
