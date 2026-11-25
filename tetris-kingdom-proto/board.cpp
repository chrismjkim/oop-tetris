#include "board.h"
#include "blocks.h"

#include <algorithm>

namespace tetris {

void refresh_void_column(const Board& board, VoidMap& voids, int x)
{
    bool above = false;
    for (int y = 0; y < kHeight; ++y) {
        voids[y][x] = above && board[y][x].durability == 0;
        above = above || board[y][x].durability != 0;
    }
    bool below = false;
    for (int y = kHeight - 1; y >= 0; --y) {
        voids[y][x] = voids[y][x] && (y == kHeight - 1 ? board[y - 1][x].durability != 0 : below);
        below = below || board[y][x].durability != 0;
    }
}

namespace {
bool filled_row(const std::array<Cell, kWidth>& row)
{
    return std::all_of(row.begin(), row.end(), [](Cell cell) { return cell.durability != 0; });
}
}

bool has_full_line(const Board& board)
{
    return std::any_of(board.begin(), board.end(), filled_row);
}

bool strike_check(const Board& board, const Piece& piece)
{
    const auto cells = piece_cells(piece);
    for (int i = 0; i < piece_size(piece.spec); ++i) {
        const Point cell = cells[i];
        if (cell.x < 0 || cell.x >= kWidth || cell.y >= kHeight) return true;
        if (cell.y >= 0 && board[cell.y][cell.x].durability > 0) return true;
    }
    return false;
}

bool merge_block(Board& board, const Piece& piece, int piece_id)
{
    if (strike_check(board, piece)) return false;
    const auto cells = piece_cells(piece);
    for (int i = 0; i < piece_size(piece.spec); ++i) {
        if (cells[i].y < 0) return false;
    }
    for (int i = 0; i < piece_size(piece.spec); ++i) {
        const Point cell = cells[i];
        board[cell.y][cell.x] = {piece_id, i == piece.spec.reinforced ? 2 : 1};
    }
    return true;
}

ClearResult remove_full_lines(Board& board)
{
    ClearResult result;
    std::array<bool, kHeight> full{};
    for (int y = 0; y < kHeight; ++y) {
        full[y] = filled_row(board[y]);
        if (full[y]) ++result.lines;
    }
    if (result.lines == 0) return result;

    // 한 판정을 시작할 때 완성된 줄을 고정하고, 각 열을 아래부터 압축한다.
    // 강화 칸이 남은 줄은 해당 열에서 한 칸을 그대로 차지한다.
    for (int x = 0; x < kWidth; ++x) {
        int destination = kHeight - 1;
        for (int y = kHeight - 1; y >= 0; --y) {
            Cell cell = board[y][x];
            if (full[y] && cell.durability == 1) {
                result.removed.push_back({{x, y}, cell});
                continue;
            }
            if (full[y]) --cell.durability;
            board[destination--][x] = cell;
        }
        for (int y = destination; y >= 0; --y) board[y][x] = {};
    }
    return result;
}

} // namespace tetris
