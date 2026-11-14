#include "BlockManager.h"

#include <cstdlib>

void BlockManager::reset()
{
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            total_block[row][column] = row == rows - 1 || column == 0 || column == columns - 1;
        }
    }
}

int BlockManager::make_new_block(int stick_rate) const
{
    if (std::rand() % 100 <= stick_rate) {
        return 0;
    }
    return std::rand() % (Block::shape_count - 1) + 1;
}

bool BlockManager::occupied(int row, int column) const
{
    return total_block[row][column] == 1;
}

bool BlockManager::strike_check(const Block& piece) const
{
    for (int row = 0; row < Block::size; ++row) {
        for (int column = 0; column < Block::size; ++column) {
            if (!piece.occupied(row, column)) {
                continue;
            }

            const int board_x = piece.x + column;
            const int board_y = piece.y + row;
            if (board_x <= 0 || board_x >= columns - 1 || board_y >= rows - 1) {
                return true;
            }

            // 블록은 화면 위에서 등장하므로 음수 행은 빈 공간이다.
            if (board_y >= 0 && occupied(board_y, board_x)) {
                return true;
            }
        }
    }
    return false;
}

bool BlockManager::merge_block(const Block& piece)
{
    if (strike_check(piece)) {
        return false;
    }

    // 화면 위에 남은 칸이 있으면 게임판을 변경하기 전에 게임 오버를 알린다.
    for (int row = 0; row < Block::size && piece.y + row < 0; ++row) {
        for (int column = 0; column < Block::size; ++column) {
            if (piece.occupied(row, column)) {
                return false;
            }
        }
    }

    for (int row = 0; row < Block::size; ++row) {
        for (int column = 0; column < Block::size; ++column) {
            if (piece.occupied(row, column)) {
                total_block[piece.y + row][piece.x + column] = 1;
            }
        }
    }
    return true;
}

bool BlockManager::check_full_line(int row) const
{
    for (int column = 1; column < columns - 1; ++column) {
        if (!occupied(row, column)) {
            return false;
        }
    }
    return true;
}

void BlockManager::clear_line(int row)
{
    for (int target = row; target > 0; --target) {
        for (int column = 1; column < columns - 1; ++column) {
            total_block[target][column] = total_block[target - 1][column];
        }
    }
    for (int column = 1; column < columns - 1; ++column) {
        total_block[0][column] = 0;
    }
}
