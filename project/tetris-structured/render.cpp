#include "render.h"
#include "blocks.h"
#include "console.h"

#include <cstdio>
#include <cstdlib>

namespace tetris {
namespace render {
namespace {

// 화면 좌표는 보드의 논리 좌표와 분리한다. 블록 하나는 가로 두 칸이다.
constexpr int kOriginX = 5;
constexpr int kOriginY = 1;
constexpr int kCellWidth = 2;
const int kBlockColors[kShapeCount] = {
    console::RED, console::BLUE, console::SKY_BLUE, console::WHITE,
    console::YELLOW, console::VIOLET, console::GREEN
};

void draw_piece(const Piece& piece, bool erase)
{
    if (!erase) {
        console::set_color(kBlockColors[piece.shape]);
    }
    for (int row = 0; row < kBlockSize; ++row) {
        for (int column = 0; column < kBlockSize; ++column) {
            if (piece.y + row < 0 || !block_cell(piece.shape, piece.angle, row, column)) {
                continue;
            }
            console::gotoxy((piece.x + column) * kCellWidth + kOriginX,
                            piece.y + row + kOriginY);
            std::printf(erase ? "  " : "■");
        }
    }
    if (!erase) {
        console::set_color(console::BLACK);
        console::gotoxy(77, 23);
    }
}

} // namespace

void show_cur_block(const Piece& piece)
{
    draw_piece(piece, false);
}

void erase_cur_block(const Piece& piece)
{
    draw_piece(piece, true);
}

void show_total_block(const GameState& game)
{
    for (int row = 0; row < kBoardRows; ++row) {
        for (int column = 0; column < kBoardColumns; ++column) {
            const bool wall = column == 0 || column == kBoardColumns - 1 || row == kPlayRows;
            console::set_color(wall ? game.level % 6 + 1 : console::DARK_GRAY);
            console::gotoxy(column * kCellWidth + kOriginX, row + kOriginY);
            std::printf(game.board.cells[row][column] ? "■" : "  ");
        }
    }
    console::set_color(console::BLACK);
    console::gotoxy(77, 23);
}

void show_next_block(const GameState& game)
{
    console::set_color((game.level + 1) % 6 + 1);
    for (int row = 1; row < 7; ++row) {
        console::gotoxy(33, row);
        for (int column = 0; column < 6; ++column) {
            const bool wall = row == 1 || row == 6 || column == 0 || column == 5;
            std::printf(wall ? "■" : "  ");
        }
    }
    show_cur_block(Piece{game.next_shape, 0, 15, 1});
}

void show_gamestat(const GameState& game)
{
    console::set_color(console::GRAY);
    console::gotoxy(35, 7);
    std::printf("STAGE");
    console::gotoxy(35, 9);
    std::printf("SCORE");
    console::gotoxy(35, 12);
    std::printf("LINES");
    console::gotoxy(41, 7);
    std::printf("%d", game.level + 1);
    console::gotoxy(35, 10);
    std::printf("%10d", game.score);
    console::gotoxy(35, 13);
    std::printf("%10d", kStages[game.level].clear_line - game.lines);
}

void animate_full_line(int row)
{
    console::set_color(console::BLUE);
    for (int pass = 0; pass < 2; ++pass) {
        console::gotoxy(kCellWidth + kOriginX, row + kOriginY);
        for (int column = 1; column < kBoardColumns - 1; ++column) {
            std::printf(pass == 0 ? "□" : "  ");
            console::sleep_ms(10);
        }
    }
}

void show_gameover()
{
    const char* frame[] = {
        "┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
        "┃**************************┃",
        "┃*        GAME OVER       *┃",
        "┃**************************┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    console::set_color(console::RED);
    for (int row = 0; row < 5; ++row) {
        console::gotoxy(15, 8 + row);
        std::printf("%s", frame[row]);
    }
}

void show_controls()
{
    const char* text[] = {
        "┏━━━━━━━━━<GAME KEY>━━━━━━━━━┓",
        "┃ UP   : Rotate Block        ┃",
        "┃ DOWN : Move One-Step Down  ┃",
        "┃ SPACE: Move Bottom Down    ┃",
        "┃ LEFT : Move Left           ┃",
        "┃ RIGHT: Move Right          ┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    console::set_color(console::GRAY);
    for (int row = 0; row < 7; ++row) {
        console::gotoxy(10, 7 + row);
        std::printf("%s", text[row]);
        console::sleep_ms(10);
    }
}

void show_level_prompt()
{
    console::gotoxy(10, 3);
    std::printf("Select Start level[1-8]:               ");
    console::gotoxy(34, 3);
    std::fflush(stdout);
}

void show_logo()
{
    const char* logo[] = {
        "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
        "┃◆◆◆  ◆◆◆  ◆◆◆   ◆◆     ◆   ◆◆◆ ┃",
        "┃  ◆    ◆        ◆     ◆ ◆    ◆   ◆     ┃",
        "┃  ◆    ◆◆◆    ◆     ◆◆     ◆     ◆   ┃",
        "┃  ◆    ◆        ◆     ◆ ◆    ◆       ◆ ┃",
        "┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    console::set_color(console::GRAY);
    for (int row = 0; row < 7; ++row) {
        console::gotoxy(13, 3 + row);
        std::printf("%s", logo[row]);
        console::sleep_ms(100);
    }
    console::gotoxy(28, 20);
    std::printf("Please Press Any Key~!");
}

void animate_logo()
{
    for (int row = 0; row < 5; ++row) {
        console::gotoxy(18, 14 + row);
        std::printf("                                                          ");
    }
    const int positions[] = {6, 12, 19, 24};
    for (int x : positions) {
        show_cur_block(Piece{std::rand() % kShapeCount, std::rand() % kRotationCount, x, 14});
    }
}

} // namespace render
} // namespace tetris
