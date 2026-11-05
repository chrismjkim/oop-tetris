#pragma once

#include "game_types.h"

namespace tetris {

bool block_cell(int shape, int angle, int row, int column);
int make_new_block(int stick_rate);
Piece make_start_piece(int shape);

} // namespace tetris
