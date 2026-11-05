#pragma once

#include "game_types.h"

namespace tetris {
namespace render {

void show_cur_block(const Piece& piece);
void erase_cur_block(const Piece& piece);
void show_total_block(const GameState& game);
void show_next_block(const GameState& game);
void show_gamestat(const GameState& game);
void animate_full_line(int row);
void show_gameover();
void show_logo();
void animate_logo();
void show_controls();
void show_level_prompt();

} // namespace render
} // namespace tetris
