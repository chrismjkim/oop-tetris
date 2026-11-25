#include "render.h"
#include "blocks.h"
#include "mechanics.h"
#include "console.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>

#define TETRIS_STRING_VALUE_IMPL(x) #x
#define TETRIS_STRING_VALUE(x) TETRIS_STRING_VALUE_IMPL(x)
#ifndef TETRIS_VERSION
#define TETRIS_VERSION dev
#endif

namespace tetris {
namespace render {
namespace {

constexpr int kHoldX = 1;
constexpr int kBoardY = 7;
constexpr int kBoardWidth = (kWidth + 2) * 2;
constexpr int kBoardRows = kHeight + 1;
constexpr int kPreviewWidth = 12;
constexpr int kPanelX = 55;
constexpr int kArtWidth = 52;
constexpr int kFooterY = kBoardY + kBoardRows;
constexpr int kWallColor = console::DARK_GRAY;
constexpr const char* kChapterNames[kChapterCount] = {"Village", "Plaza", "Clock Tower", "Palace"};


constexpr char kChapterArt[kChapterCount][kBoardRows][kArtWidth + 1] = {
    {
        "##                                                ##",
        "##                                                ##",
        "##                                                ##",
        "##                                    ~  ~~~  ~   ##",
        "##                                                ##",
        "##    __.--.__                                    ##",
        "##       ~~~                                      ##",
        "##                       /\\                 []    ##",
        "##       /\\             /  \\            /\\  ||    ##",
        "##      /  \\           /____\\          /  \\ ||    ##",
        "##     /____\\         /______\\        /____\\||    ##",
        "##    /______\\       /________\\      /______\\|    ##",
        "##   /________\\    /____________\\   /________\\    ##",
        "## /____________\\  | []  o  []  |  /____________\\ ##",
        "## | []      [] |  |     __     |  |  []    []  | ##",
        "## |     __     |  | [] |  | [] |  |     __     | ##",
        "## |    |  |    |  |    |  |    |  |    |  |    | ##",
        "## |____|__|____|  |____|__|____|  |____|__|____| ##",
        "## |============|      /    \\      |============| ##",
        "##    ..    ..        /______\\        ..    ..    ##",
        "####################################################",
    },
    {
        "##                                                ##",
        "##                                                ##",
        "##                                                ##",
        "##                                                ##",
        "##                  .    :    .                   ##",
        "##                   '.  :  .'                    ##",
        "##     /\\             \\  :  /             /\\      ##",
        "##    /==\\           .-\\-^-/-.           /==\\     ##",
        "##    |oo|           ( ~~~~~ )           |oo|     ##",
        "##    '--'           '---+---'           '--'     ##",
        "##     ||               /|\\               ||      ##",
        "##     ||              /_|_\\              ||      ##",
        "##     ||        .------' '------.        ||      ##",
        "##     ||       / ~   ~   ~   ~   \\       ||      ##",
        "##    _||_    ( ~   ~   ~   ~   ~   )    _||_     ##",
        "##   |____|    '-------------------'    |____|    ##",
        "##           [_______________________]            ##",
        "##          |_________________________|           ##",
        "##-----+-----+-----+-----+-----+-----+-----+-----+##",
        "##  ___|  ___|  ___|  ___|  ___|  ___|  ___|  ___|##",
        "####################################################",
    },
    {
        "##                       |>>>>                    ##",
        "##                       |                        ##",
        "##                       /\\                       ##",
        "##                      /  \\                      ##",
        "##                     /____\\                     ##",
        "##                    /______\\                    ##",
        "##                  /__________\\                  ##",
        "##                /______________\\                ##",
        "##                |==============|                ##",
        "##                | .----------. |                ##",
        "##                | |    12    | |                ##",
        "##                | |9   o--  3| |                ##",
        "##                | |    6     | |                ##",
        "##                | '----------' |                ##",
        "##                |==============|                ##",
        "##                |  []      []  |                ##",
        "##     /\\         |     ____     |           /\\   ##",
        "##    /  \\        |____/    \\____|          /  \\  ##",
        "##   /____\\    +--------------------+      /____\\ ##",
        "##     ||     +----------------------+       ||   ##",
        "####################################################",
    },
    {
        "##                       |====>                   ##",
        "##                       |                        ##",
        "##                       /\\                       ##",
        "##    |>>               /  \\              |>>     ##",
        "##    |                /____\\             |       ##",
        "##     /\\             /______\\             /\\     ##",
        "##    /  \\           /________\\           /  \\    ##",
        "##   /____\\          | []  [] |          /____\\   ##",
        "##  /______\\         |   ::   |         /______\\  ##",
        "## /________\\        |________|        /________\\ ##",
        "## |_[]__[]_|     |[]  []  []  []|     |_[]__[]_| ##",
        "## |        |_[ ]_|--------------|_[ ]_|        | ##",
        "## |   []   |_____|   .------.   |_____|   []   | ##",
        "## |        |     |  /        \\  |     |        | ##",
        "## |   []   | []  | |  |  |  | | |  [] |   []   | ##",
        "## |        |     | |  |  |  | | |     |        | ##",
        "## |        | []  | |  |  |  | | |  [] |        | ##",
        "## |________|_____|_|__|__|__|_|_|_____|________| ##",
        "##  +==========================================+  ##",
        "## +____________________________________________+ ##",
        "####################################################",
    },
};

constexpr int kColors[kShapeCount] = {
    console::BLOCK_I, console::BLOCK_O, console::BLOCK_T,
    console::BLOCK_J, console::BLOCK_L, console::BLOCK_Z, console::BLOCK_S
};

void text(int x, int y, int color, const char* format, ...)
{
    char buffer[160];
    va_list arguments;
    va_start(arguments, format);
    std::vsnprintf(buffer, sizeof(buffer), format, arguments);
    va_end(arguments);
    console::write_text(x, y, color, buffer);
}

void draw_active(const Piece& piece, int x, bool ghost)
{
    const auto coordinates = piece_cells(piece);
    const int color = ghost ? console::DARK_GRAY : kColors[piece.spec.shape];
    for (int i = 0; i < piece_size(piece.spec); ++i) {
        const Point point = coordinates[i];
        if (point.y < 0 || point.y >= kHeight) continue;
        const int sx = x + (point.x + 1) * 2;
        const auto style = !ghost && i == piece.spec.reinforced
            ? console::BlockStyle::Reinforced : console::BlockStyle::Solid;
        console::write_block(sx, kBoardY + point.y, color, style);
    }
}

void preview(int x, const char* name, const PieceSpec* spec)
{
    const int height = spec && spec->giant ? 8 : 5;
    const char* shape = spec ? kShapeNames[spec->shape] : "-";
    text(x, kBoardY, console::GRAY, "+- %s %s -+", name, shape);
    for (int y = 1; y <= height; ++y) {
        text(x, kBoardY + y, console::GRAY, "|");
        text(x + kPreviewWidth - 1, kBoardY + y, console::GRAY, "|");
    }
    text(x, kBoardY + height + 1, console::GRAY, "+----------+");
    if (!spec) return;
    const auto cells = piece_cells(Piece{*spec, {}, 0});
    const int color = kColors[spec->shape];
    for (int i = 0; i < piece_size(*spec); ++i) {
        const Point point = cells[i];
        const bool reinforced = i == spec->reinforced;
        if (spec->giant) {
            const int sx = x + 2 + point.x, sy = kBoardY + 1 + point.y;
            if (reinforced) console::write_block(sx, sy, color, console::BlockStyle::CompactReinforced);
            else text(sx, sy, color, "#");
        } else {
            console::write_block(x + 3 + point.x * 2, kBoardY + 2 + point.y, color,
                reinforced ? console::BlockStyle::Reinforced : console::BlockStyle::Solid);
        }
    }
}

void board(const GameState& game, int x)
{
    for (int y = 0; y < kHeight; ++y) {
        const int sy = kBoardY + y;
        console::write_block(x, sy, kWallColor);
        console::write_block(x + (kWidth + 1) * 2, sy, kWallColor);
        for (int col = 0; col < kWidth; ++col) {
            const int sx = x + (col + 1) * 2;
            const Cell cell = game.board[y][col];
            if (cell.durability == 0) {
                text(sx, sy, console::DARK_GRAY, " .");
                continue;
            }
            const auto entry = game.placed.find(cell.piece_id);
            const int color = entry != game.placed.end() ? kColors[entry->second.spec.shape] : console::GRAY;
            if (cell.piece_id == game.warning_id && game.warning_id != 0) {
                text(sx, sy, color, "##");
            } else {
                console::write_block(sx, sy, color,
                    cell.durability == 2 ? console::BlockStyle::Reinforced : console::BlockStyle::Solid);
            }
        }
    }
    for (int sx = 0; sx < kBoardWidth; sx += 2) {
        console::write_block(x + sx, kBoardY + kHeight, kWallColor);
    }

    if (game.phase == Phase::TheftAnimation) {
        for (const RemovedCell& cell : game.stolen_cells) {
            text(x + (cell.position.x + 1) * 2, kBoardY + cell.position.y,
                kColors[game.stolen_shape], "xx");
        }
    }
    if (game.phase != Phase::Playing) return;
    draw_active(landing_projection(game), x, true);
    draw_active(game.current, x, false);
}

const char* thief_status(const GameState& game)
{
    switch (game.theft_outcome) {
    case TheftOutcome::Warned: return "THIEF TARGET: ##";
    case TheftOutcome::Cancelled: return "THEFT CANCELLED";
    case TheftOutcome::Stolen: return "BLOCK STOLEN";
    default: return "";
    }
}

void field(const GameState& game, int x)
{
    const int board_x = x + kPreviewWidth;
    preview(x, "HOLD", game.has_hold ? &game.hold : nullptr);
    board(game, board_x);
    preview(board_x + kBoardWidth, "NEXT", &game.next);
    text(x, 5, console::YELLOW, "%s", thief_status(game));
    if (game.phase == Phase::Choosing) {
        text(x, 4, console::YELLOW, "CHOOSE AUGMENT");
        for (int i = 0; i < 3; ++i) {
            const Card card = game.augments.choices[i];
            const CardInfo& info = card_info(card.kind);
            const bool selected = game.augments.cursor == i;
            const int color = selected ? console::YELLOW : console::GRAY;
            const int top = kBoardY + 1 + i * 6;
            for (int row = 0; row < 5; ++row) {
                const bool edge = row == 0 || row == 4;
                text(board_x, top + row, color, "%s", edge
                    ? (selected ? "+======================+" : "+----------------------+")
                    : "|                      |");
            }
            std::wstring name;
            if (card.shape >= 0) {
                name += static_cast<wchar_t>(kShapeNames[card.shape][0]);
                name += L" ";
            }
            name += info.name;
            console::write_wide_text(board_x + 1, top + 1, color, name.c_str());
            console::write_wide_text(board_x + 1, top + 2, color, info.line1);
            console::write_wide_text(board_x + 1, top + 3, color, info.line2);
        }
    }
}

void construction(const GameState& game)
{
    int chapter = game.level;
    if (game.phase == Phase::Choosing && game.augments.next_choice > 0 &&
        game.augments.next_choice % game.chapter_goal == 0) {
        chapter = game.augments.next_choice / game.chapter_goal - 1;
    }
    const int lines = game.total_lines - chapter * game.chapter_goal;
    const int rows = std::clamp(lines * kHeight / game.chapter_goal, 0, kHeight);
    for (int y = 0; y < kHeight; ++y) {
        console::write_block(kPanelX, kBoardY + y, kWallColor);
        console::write_block(kPanelX + kArtWidth - 2, kBoardY + y, kWallColor);
        if (y >= kHeight - rows) {
            text(kPanelX + 2, kBoardY + y, console::GRAY, "%.*s",
                kArtWidth - 4, kChapterArt[chapter][y] + 2);
        }
    }
    for (int x = 0; x < kArtWidth; x += 2) {
        console::write_block(kPanelX + x, kBoardY + kHeight, kWallColor);
    }
}

void two_player_field(const GameState& game, int x, int player)
{
    text(x, 0, console::WHITE, "%dP  SCORE %lld  LINES %d", player,
        static_cast<long long>(game.score), game.total_lines);
    field(game, x);
}

} // namespace

void show_game(const GameState& game, bool help)
{
    console::begin_frame();
    text(kHoldX, 0, console::WHITE, "SCORE %lld", static_cast<long long>(game.score));
    if (game.mode == Mode::Story) {
        text(22, 0, console::WHITE, "LINES %d/%d", game.total_lines, kChapterCount * game.chapter_goal);
        text(42, 0, console::WHITE, "STAGE %d/%d %s", game.level + 1, kChapterCount, kChapterNames[game.level]);
        construction(game);
    } else {
        text(22, 0, console::WHITE, "LINES %d", game.total_lines);
        text(42, 0, console::WHITE, "ENDLESS / LEVEL %d", game.level + 1);
    }
    field(game, kHoldX);
    if (help) text(kPanelX, 3, console::YELLOW, "PAUSED / H: resume");
    if (game.phase == Phase::GameOver) text(kPanelX, 1, console::YELLOW, "GAME OVER / R: restart");
    if (game.phase == Phase::Victory) text(kPanelX, 1, console::YELLOW, "KINGDOM REBUILT / R: restart");
    text(kHoldX, kFooterY, console::GRAY, "%s", game.phase == Phase::Choosing
        ? "Up/Down: select augment  SPACE: confirm  ESC: menu"
        : "Arrows: move/rotate/drop  SPACE: hard drop  C: hold  H: pause  ESC: menu");
    console::write_wide_text(kHoldX, kFooterY + 1, console::GRAY,
        L"\u25A3: reinforced  ##: thief target  Giant cannot be held");
    console::end_frame();
}

void show_two_player_game(const GameState& first, const GameState& second)
{
    console::begin_frame();
    two_player_field(first, kHoldX, 1);
    two_player_field(second, kPanelX, 2);
    text(kHoldX, 2, console::WHITE, "LOCAL 2P  /  LEVEL %d", first.level + 1);
    if (first.phase == Phase::GameOver && second.phase == Phase::GameOver) {
        text(kHoldX, 3, console::YELLOW, "GAME OVER / R: restart both boards");
    }
    text(kHoldX, kFooterY, console::GRAY, "%s", first.phase == Phase::Choosing
        ? "1P I/K: select  Space/S: confirm" : "1P I/J/K/L, Shift/W, Space/S");
    text(kPanelX, kFooterY, console::GRAY, "%s", second.phase == Phase::Choosing
        ? "2P numpad 5/2: select  Down: confirm" : "2P numpad 5/1/2/3, Up/Down");
    console::write_wide_text(kHoldX, kFooterY + 1, console::GRAY,
        L"ESC: menu  \u25A3: reinforced  ##: thief target");
    console::end_frame();
}

void show_main_menu()
{
    constexpr int x = console::kFrameColumns / 2 - 19;
    console::begin_frame();
    text(x, 8, console::WHITE, "BLOCK KINGDOM REBUILD  " TETRIS_STRING_VALUE(TETRIS_VERSION));
    text(x, 11, console::SKY_BLUE, "[1]  1P Story");
    text(x, 13, console::GREEN, "[2]  Local 2P");
    text(x, 15, console::GREEN, "[3]  1P Endless");
    text(x, 18, console::GRAY, "ESC  Quit");
    console::end_frame();
}

} // namespace render
} // namespace tetris
