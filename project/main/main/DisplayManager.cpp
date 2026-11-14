#include "DisplayManager.h"

#include "BlockManager.h"
#include "StageManager.h"

#include <Windows.h>
#include <cstdio>
#include <cstdlib>

namespace {

constexpr int offset_x = 5;
constexpr int offset_y = 1;
constexpr int black = 0;
constexpr int blue = 9;
constexpr int gray = 7;
constexpr int dark_gray = 8;
constexpr int red = 12;
constexpr int block_colors[Block::shape_count] = {red, blue, 11, 15, 14, 13, 10};

void gotoxy(int x, int y)
{
    std::fflush(stdout);
    const COORD position = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
}

void SetColor(int color)
{
    std::fflush(stdout);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
}

void draw_block(const Block& piece, const char* cell)
{
    for (int column = 0; column < Block::size; ++column) {
        for (int row = 0; row < Block::size; ++row) {
            if (piece.y + row >= 0 && piece.occupied(row, column)) {
                gotoxy((piece.x + column) * 2 + offset_x, piece.y + row + offset_y);
                std::printf("%s", cell);
            }
        }
    }
}

} // namespace

void DisplayManager::initialize_console() const
{
    SetConsoleOutputCP(CP_UTF8);
}

void DisplayManager::show_logo() const
{
    const char* title[] = {
        "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
        "┃◆◆◆  ◆◆◆  ◆◆◆   ◆◆     ◆   ◆◆◆ ┃",
        "┃  ◆    ◆        ◆     ◆ ◆    ◆   ◆     ┃",
        "┃  ◆    ◆◆◆    ◆     ◆◆     ◆     ◆   ┃",
        "┃  ◆    ◆        ◆     ◆ ◆    ◆       ◆ ┃",
        "┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    for (int row = 0; row < 7; ++row) {
        gotoxy(13, 3 + row);
        std::printf("%s", title[row]);
        if (row < 6) {
            wait(100);
        }
    }
    gotoxy(28, 20);
    std::printf("Please Press Any Key~!");
}

void DisplayManager::animate_logo() const
{
    for (int row = 0; row < 5; ++row) {
        gotoxy(18, 14 + row);
        std::printf("%58s", "");
    }
    const int positions[] = {6, 12, 19, 24};
    for (int x : positions) {
        Block piece;
        piece.shape = std::rand() % Block::shape_count;
        piece.angle = std::rand() % Block::size;
        piece.x = x;
        piece.y = 14;
        show_cur_block(piece);
    }
}

void DisplayManager::show_controls() const
{
    const char* controls[] = {
        "┏━━━━━━━━━<GAME KEY>━━━━━━━━━┓",
        "┃ UP   : Rotate Block        ┃",
        "┃ DOWN : Move One-Step Down  ┃",
        "┃ SPACE: Move Bottom Down    ┃",
        "┃ LEFT : Move Left           ┃",
        "┃ RIGHT: Move Right          ┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    SetColor(gray);
    for (int row = 0; row < 7; ++row) {
        gotoxy(10, 7 + row);
        std::printf("%s", controls[row]);
        if (row < 6) {
            wait(10);
        }
    }
}

void DisplayManager::show_level_prompt() const
{
    gotoxy(10, 3);
    std::printf("Select Start level[1-8]:       \b\b\b\b\b\b");
    std::fflush(stdout);
}

void DisplayManager::show_cur_block(const Block& piece) const
{
    SetColor(block_colors[piece.shape]);
    draw_block(piece, "■");
    SetColor(black);
    park_cursor();
}

void DisplayManager::erase_cur_block(const Block& piece) const
{
    draw_block(piece, "  ");
}

void DisplayManager::show_total_block(const BlockManager& board, int level) const
{
    for (int row = 0; row < BlockManager::rows; ++row) {
        for (int column = 0; column < BlockManager::columns; ++column) {
            const bool border = column == 0 || column == BlockManager::columns - 1
                || row == BlockManager::rows - 1;
            SetColor(border ? level % 6 + 1 : dark_gray);
            gotoxy(column * 2 + offset_x, row + offset_y);
            std::printf("%s", board.occupied(row, column) ? "■" : "  ");
        }
    }
    SetColor(black);
    park_cursor();
}

void DisplayManager::show_next_block(int shape, int level) const
{
    SetColor((level + 1) % 6 + 1);
    for (int row = 1; row < 7; ++row) {
        gotoxy(33, row);
        for (int column = 0; column < 6; ++column) {
            const bool border = row == 1 || row == 6 || column == 0 || column == 5;
            std::printf("%s", border ? "■" : "  ");
        }
    }

    Block preview;
    preview.shape = shape;
    preview.x = 15;
    preview.y = 1;
    show_cur_block(preview);
}

void DisplayManager::show_gamestat(const StageManager& stage) const
{
    SetColor(gray);
    gotoxy(35, 7);
    std::printf("STAGE %d", stage.level + 1);
    gotoxy(35, 9);
    std::printf("SCORE");
    gotoxy(35, 12);
    std::printf("LINES");
    gotoxy(35, 10);
    std::printf("%10d", stage.score);
    gotoxy(35, 13);
    std::printf("%10d", stage.clear_line() - stage.lines);
}

void DisplayManager::animate_clear_line(int row) const
{
    SetColor(blue);
    const char* animation[] = {"□", "  "};
    for (const char* cell : animation) {
        gotoxy(2 + offset_x, row + offset_y);
        for (int column = 1; column < BlockManager::columns - 1; ++column) {
            std::printf("%s", cell);
            wait(10);
        }
    }
}

void DisplayManager::show_gameover() const
{
    const char* message[] = {
        "┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
        "┃**************************┃",
        "┃*        GAME OVER       *┃",
        "┃**************************┃",
        "┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
    };
    SetColor(red);
    for (int row = 0; row < 5; ++row) {
        gotoxy(15, 8 + row);
        std::printf("%s", message[row]);
    }
    wait(1000);
    SetColor(gray);
}

void DisplayManager::clear() const
{
    SetColor(gray);
    std::system("cls");
}

void DisplayManager::wait(int milliseconds) const
{
    std::fflush(stdout);
    Sleep(static_cast<DWORD>(milliseconds));
}

void DisplayManager::park_cursor() const
{
    gotoxy(77, 23);
}
