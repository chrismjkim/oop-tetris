#include "../blocks.h"
#include "../board.h"

#include <cassert>
#include <cstdio>
#include <cstring>

using namespace tetris;

namespace {

int occupied_cells(const Board& board)
{
    int count = 0;
    for (int row = 0; row < kPlayRows; ++row) {
        for (int column = 1; column < kBoardColumns - 1; ++column) {
            count += board.cells[row][column] != 0;
        }
    }
    return count;
}

void assert_walls(const Board& board)
{
    for (int row = 0; row < kBoardRows; ++row) {
        assert(board.cells[row][0] == 1);
        assert(board.cells[row][kBoardColumns - 1] == 1);
    }
    for (int column = 0; column < kBoardColumns; ++column) {
        assert(board.cells[kPlayRows][column] == 1);
    }
}

void test_shapes_and_boundaries()
{
    Board board;
    reset_board(board);
    assert_walls(board);
    assert(occupied_cells(board) == 0);
    assert(!strike_check(board, Piece{1, 0, 1, 18}));
    assert(!strike_check(board, Piece{1, 0, 11, 18}));
    assert(strike_check(board, Piece{1, 0, 0, 18}));
    assert(strike_check(board, Piece{1, 0, 12, 18}));
    assert(strike_check(board, Piece{1, 0, 5, 19}));

    for (int shape = 0; shape < kShapeCount; ++shape) {
        const Piece spawn = make_start_piece(shape);
        assert(spawn.shape == shape && spawn.angle == 0 && spawn.x == 5 && spawn.y == -3);
        assert(!strike_check(board, spawn));
        for (int angle = 0; angle < kRotationCount; ++angle) {
            int count = 0;
            for (int row = 0; row < kBlockSize; ++row) {
                for (int column = 0; column < kBlockSize; ++column) {
                    count += block_cell(shape, angle, row, column);
                }
            }
            assert(count == 4);
            // AddressSanitizer로 모든 회전의 화면 밖/벽/바닥 접근을 검사한다.
            for (int y = -4; y <= kBoardRows + 2; ++y) {
                for (int x = -4; x <= kBoardColumns + 2; ++x) {
                    reset_board(board);
                    const Piece piece{shape, angle, x, y};
                    const bool collision = strike_check(board, piece);
                    const bool merged = merge_block(board, piece);
                    assert(!collision || !merged);
                    assert(occupied_cells(board) == (merged ? 4 : 0));
                    assert_walls(board);
                }
            }
        }
    }
}

void test_locking()
{
    Board board;
    reset_board(board);
    // 가로 막대의 빈 행/열은 보드 바깥에 있어도 읽거나 쓰지 않아야 한다.
    const Piece bottom_bar{0, 1, 9, 19};
    assert(merge_block(board, bottom_bar));
    assert(occupied_cells(board) == 4);
    assert(strike_check(board, bottom_bar));
    assert(!merge_block(board, bottom_bar));
    assert_walls(board);

    reset_board(board);
    board.cells[3][5] = 1;
    const Piece partly_above_top{0, 0, 5, -1};
    assert(!strike_check(board, partly_above_top));
    assert(strike_check(board, Piece{0, 0, 5, 0}));
    const Board before = board;
    assert(!merge_block(board, partly_above_top));
    assert(std::memcmp(before.cells, board.cells, sizeof(board.cells)) == 0);
}

void test_line_removal()
{
    Board board;
    reset_board(board);
    board.cells[17][3] = 1;
    for (int column = 1; column < kBoardColumns - 1; ++column) {
        board.cells[18][column] = 1;
        board.cells[19][column] = 1;
    }
    assert(!is_full_line(board, 17));
    assert(is_full_line(board, 18) && is_full_line(board, 19));
    remove_line(board, 18);
    assert(board.cells[18][3] == 1);
    assert(is_full_line(board, 19));
    remove_line(board, 19);
    assert(board.cells[19][3] == 1);
    assert(occupied_cells(board) == 1);
    assert_walls(board);

    for (int column = 1; column < kBoardColumns - 1; ++column) {
        board.cells[0][column] = 1;
    }
    remove_line(board, 0);
    for (int column = 1; column < kBoardColumns - 1; ++column) {
        assert(board.cells[0][column] == 0);
    }
    assert_walls(board);
}

} // namespace

int main()
{
    test_shapes_and_boundaries();
    test_locking();
    test_line_removal();
    std::puts("PASS: shapes, boundaries, locking, top-out and line removal");
}
