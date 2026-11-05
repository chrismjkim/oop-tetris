#pragma once

#include "game_types.h"

namespace tetris {

void reset_board(Board& board);
bool strike_check(const Board& board, const Piece& piece);
// 화면 위에 남은 블록은 고정하지 않고 false를 반환한다(게임 오버).
bool merge_block(Board& board, const Piece& piece);
bool is_full_line(const Board& board, int row);
void remove_line(Board& board, int row);

} // namespace tetris
