#pragma once

#include "game_types.h"

namespace tetris {

int piece_size(const PieceSpec& spec);
std::array<Point, 16> piece_cells(const Piece& piece);
Piece spawn_at_top(PieceSpec spec);
PieceSpec draw_piece(SevenBag& bag, std::mt19937& random, const GimmickRules& rules);

} // namespace tetris
