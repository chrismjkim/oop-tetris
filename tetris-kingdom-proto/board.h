#pragma once

#include "game_types.h"

namespace tetris {

bool strike_check(const Board& board, const Piece& piece);
bool merge_block(Board& board, const Piece& piece, int piece_id);
ClearResult remove_full_lines(Board& board);
void refresh_void_column(const Board& board, VoidMap& voids, int x);
bool has_full_line(const Board& board);

} // namespace tetris
