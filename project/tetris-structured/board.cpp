#include "board.h"
#include "blocks.h"

namespace tetris {

void reset_board(Board& board)
{
    for (int row = 0; row < kBoardRows; ++row) {
        for (int column = 0; column < kBoardColumns; ++column) {
            board.cells[row][column] = static_cast<char>(
                column == 0 || column == kBoardColumns - 1 || row == kPlayRows);
        }
    }
}

bool strike_check(const Board& board, const Piece& piece)
{
    for (int row = 0; row < kBlockSize; ++row) {
        for (int column = 0; column < kBlockSize; ++column) {
            if (!block_cell(piece.shape, piece.angle, row, column)) {
                continue;
            }
            const int x = piece.x + column;
            const int y = piece.y + row;
            // 빈 칸은 읽지 않는다. 등장 중인 블록은 화면 위에 있을 수 있다.
            if (x <= 0 || x >= kBoardColumns - 1 || y >= kPlayRows) {
                return true;
            }
            if (y >= 0 && board.cells[y][x] != 0) {
                return true;
            }
        }
    }
    return false;
}

bool merge_block(Board& board, const Piece& piece)
{
    if (strike_check(board, piece)) {
        return false;
    }
    // 모두 들어갈 수 있는지 먼저 확인해 보드를 일부만 변경하지 않는다.
    for (int row = 0; row < kBlockSize; ++row) {
        for (int column = 0; column < kBlockSize; ++column) {
            if (block_cell(piece.shape, piece.angle, row, column) && piece.y + row < 0) {
                return false;
            }
        }
    }
    for (int row = 0; row < kBlockSize; ++row) {
        for (int column = 0; column < kBlockSize; ++column) {
            if (block_cell(piece.shape, piece.angle, row, column)) {
                board.cells[piece.y + row][piece.x + column] = 1;
            }
        }
    }
    return true;
}

bool is_full_line(const Board& board, int row)
{
    for (int column = 1; column < kBoardColumns - 1; ++column) {
        if (board.cells[row][column] == 0) {
            return false;
        }
    }
    return true;
}

void remove_line(Board& board, int row)
{
    for (int destination = row; destination > 0; --destination) {
        for (int column = 1; column < kBoardColumns - 1; ++column) {
            board.cells[destination][column] = board.cells[destination - 1][column];
        }
    }
    for (int column = 1; column < kBoardColumns - 1; ++column) {
        board.cells[0][column] = 0;
    }
}

} // namespace tetris
