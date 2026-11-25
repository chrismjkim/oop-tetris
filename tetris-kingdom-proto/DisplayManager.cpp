#include "DisplayManager.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>

using namespace std;

#define TETRIS_STRING_VALUE_IMPL(x) #x
#define TETRIS_STRING_VALUE(x) TETRIS_STRING_VALUE_IMPL(x)
#ifndef TETRIS_VERSION
#define TETRIS_VERSION dev
#endif

void DisplayManager::text(int x, int y, int color, const char* format, ...)
{
    char buffer[160];
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(buffer, sizeof(buffer), format, arguments);
    va_end(arguments);
    screen.writeText(x, y, color, buffer);
}

void DisplayManager::drawActive(const Block& piece, int x, bool ghost)
{
    const auto coordinates = piece.cells();
    const int color = ghost ? ConsoleScreen::DARK_GRAY : (ConsoleScreen::BLOCK_I + piece.specification().getShape());
    for (int i = 0; i < piece.specification().size(); ++i) {
        const Point point = coordinates[i];
        if (point.getY() < 0 || point.getY() >= Board::height) continue;
        const int sx = x + (point.getX() + 1) * 2;
        const auto style = !ghost && i == piece.specification().getReinforced()
            ? ConsoleScreen::BlockStyle::Reinforced : ConsoleScreen::BlockStyle::Solid;
        screen.writeBlock(sx, kBoardY + point.getY(), color, style);
    }
}

void DisplayManager::preview(int x, const char* name, const BlockSpec* spec)
{
    const int height = spec && spec->getGiant() ? 8 : 5;
    const char* shape = spec ? Block::shapeName(spec->getShape()) : "-";
    text(x, kBoardY, ConsoleScreen::GRAY, "+- %s %s -+", name, shape);
    for (int y = 1; y <= height; ++y) {
        text(x, kBoardY + y, ConsoleScreen::GRAY, "|");
        text(x + kPreviewWidth - 1, kBoardY + y, ConsoleScreen::GRAY, "|");
    }
    text(x, kBoardY + height + 1, ConsoleScreen::GRAY, "+----------+");
    if (!spec) return;
    const auto cells = (Block{*spec, {}, 0}).cells();
    const int color = (ConsoleScreen::BLOCK_I + spec->getShape());
    for (int i = 0; i < spec->size(); ++i) {
        const Point point = cells[i];
        const bool reinforced = i == spec->getReinforced();
        if (spec->getGiant()) {
            const int sx = x + 2 + point.getX(), sy = kBoardY + 1 + point.getY();
            if (reinforced) screen.writeBlock(sx, sy, color, ConsoleScreen::BlockStyle::CompactReinforced);
            else text(sx, sy, color, "#");
        } else {
            screen.writeBlock(x + 3 + point.getX() * 2, kBoardY + 2 + point.getY(), color,
                reinforced ? ConsoleScreen::BlockStyle::Reinforced : ConsoleScreen::BlockStyle::Solid);
        }
    }
}

void DisplayManager::board(const StageManager& game, int x)
{
    for (int y = 0; y < Board::height; ++y) {
        const int sy = kBoardY + y;
        screen.writeBlock(x, sy, kWallColor);
        screen.writeBlock(x + (Board::boardWidth + 1) * 2, sy, kWallColor);
        for (int col = 0; col < Board::boardWidth; ++col) {
            const int sx = x + (col + 1) * 2;
            const Cell cell = game.getBoard().at(col, y);
            if (cell.getDurability() == 0) {
                text(sx, sy, ConsoleScreen::DARK_GRAY, " .");
                continue;
            }
            const int shape = game.getBoard().shapeOf(cell.getPieceId());
            const int color = shape >= 0 ? ConsoleScreen::BLOCK_I + shape : ConsoleScreen::GRAY;
            if (cell.getPieceId() == game.theftTarget() && game.theftTarget() != 0) {
                text(sx, sy, color, "##");
            } else {
                screen.writeBlock(sx, sy, color,
                    cell.getDurability() == 2 ? ConsoleScreen::BlockStyle::Reinforced : ConsoleScreen::BlockStyle::Solid);
            }
        }
    }
    for (int sx = 0; sx < kBoardWidth; sx += 2) {
        screen.writeBlock(x + sx, kBoardY + Board::height, kWallColor);
    }

    if (game.getPhase() == StageManager::Phase::TheftAnimation) {
        for (const RemovedCell& cell : game.stolenCells()) {
            text(x + (cell.getPosition().getX() + 1) * 2, kBoardY + cell.getPosition().getY(),
                (ConsoleScreen::BLOCK_I + game.stolenShape()), "xx");
        }
    }
    if (game.getPhase() != StageManager::Phase::Playing) return;
    drawActive(game.shadow(), x, true);
    drawActive(game.activeBlock(), x, false);
}

const char* DisplayManager::thiefStatus(const StageManager& game)
{
    switch (game.theftOutcome()) {
    case StageManager::TheftOutcome::Warned: return "THIEF TARGET: ##";
    case StageManager::TheftOutcome::Cancelled: return "THEFT CANCELLED";
    case StageManager::TheftOutcome::Stolen: return "BLOCK STOLEN";
    default: return "";
    }
}

void DisplayManager::field(const StageManager& game, int x)
{
    const int board_x = x + kPreviewWidth;
    preview(x, "HOLD", game.heldBlock());
    board(game, board_x);
    preview(board_x + kBoardWidth, "NEXT", &game.nextBlock());
    text(x, 5, ConsoleScreen::YELLOW, "%s", thiefStatus(game));
    if (game.getPhase() == StageManager::Phase::Choosing) {
        text(x, 4, ConsoleScreen::YELLOW, "CHOOSE AUGMENT");
        for (int i = 0; i < 3; ++i) {
            const Card card = game.cards().choice(i);
            const CardInfo& info = Augment::info(card.getKind());
            const bool selected = game.cards().selectedIndex() == i;
            const int color = selected ? ConsoleScreen::YELLOW : ConsoleScreen::GRAY;
            const int top = kBoardY + 1 + i * 6;
            for (int row = 0; row < 5; ++row) {
                const bool edge = row == 0 || row == 4;
                text(board_x, top + row, color, "%s", edge
                    ? (selected ? "+======================+" : "+----------------------+")
                    : "|                      |");
            }
            wstring name;
            if (card.getShape() >= 0) {
                name += static_cast<wchar_t>(Block::shapeName(card.getShape())[0]);
                name += L" ";
            }
            name += info.getName();
            screen.writeWideText(board_x + 1, top + 1, color, name.c_str());
            screen.writeWideText(board_x + 1, top + 2, color, info.getLine1());
            screen.writeWideText(board_x + 1, top + 3, color, info.getLine2());
        }
    }
}

void DisplayManager::construction(const StageManager& game)
{
    static constexpr char kChapterArt[StageManager::chapterCount][kBoardRows][kArtWidth + 1] = {
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
    int chapter = game.getLevel();
    if (game.getPhase() == StageManager::Phase::Choosing && game.cards().nextChoice() > 0 &&
        game.cards().nextChoice() % game.chapterGoal() == 0) {
        chapter = game.cards().nextChoice() / game.chapterGoal() - 1;
    }
    const int lines = game.lines() - chapter * game.chapterGoal();
    const int rows = clamp(lines * Board::height / game.chapterGoal(), 0, Board::height);
    for (int y = 0; y < Board::height; ++y) {
        screen.writeBlock(kPanelX, kBoardY + y, kWallColor);
        screen.writeBlock(kPanelX + kArtWidth - 2, kBoardY + y, kWallColor);
        if (y >= Board::height - rows) {
            text(kPanelX + 2, kBoardY + y, ConsoleScreen::GRAY, "%.*s",
                kArtWidth - 4, kChapterArt[chapter][y] + 2);
        }
    }
    for (int x = 0; x < kArtWidth; x += 2) {
        screen.writeBlock(kPanelX + x, kBoardY + Board::height, kWallColor);
    }
}

void DisplayManager::twoPlayerField(const StageManager& game, int x, int player)
{
    text(x, 0, ConsoleScreen::WHITE, "%dP  SCORE %lld  LINES %d", player,
        static_cast<long long>(game.getScore()), game.lines());
    field(game, x);
}

void DisplayManager::showGame(const StageManager& game, bool help)
{
    static constexpr const char* kChapterNames[] = {"Village", "Plaza", "Clock Tower", "Palace"};
    screen.beginFrame();
    text(kHoldX, 0, ConsoleScreen::WHITE, "SCORE %lld", static_cast<long long>(game.getScore()));
    if (game.getMode() == StageManager::Mode::Story) {
        text(22, 0, ConsoleScreen::WHITE, "LINES %d/%d", game.lines(), StageManager::chapterCount * game.chapterGoal());
        text(42, 0, ConsoleScreen::WHITE, "STAGE %d/%d %s", game.getLevel() + 1, StageManager::chapterCount, kChapterNames[game.getLevel()]);
        construction(game);
    } else {
        text(22, 0, ConsoleScreen::WHITE, "LINES %d", game.lines());
        text(42, 0, ConsoleScreen::WHITE, "ENDLESS / LEVEL %d", game.getLevel() + 1);
    }
    field(game, kHoldX);
    if (help) text(kPanelX, 3, ConsoleScreen::YELLOW, "PAUSED / H: resume");
    if (game.getPhase() == StageManager::Phase::GameOver) text(kPanelX, 1, ConsoleScreen::YELLOW, "GAME OVER / R: restart");
    if (game.getPhase() == StageManager::Phase::Victory) text(kPanelX, 1, ConsoleScreen::YELLOW, "KINGDOM REBUILT / R: restart");
    text(kHoldX, kFooterY, ConsoleScreen::GRAY, "%s", game.getPhase() == StageManager::Phase::Choosing
        ? "Up/Down: select augment  SPACE: confirm  ESC: menu"
        : "Arrows: move/rotate/drop  SPACE: hard drop  C: hold  H: pause  ESC: menu");
    screen.writeWideText(kHoldX, kFooterY + 1, ConsoleScreen::GRAY,
        L"\u25A3: reinforced  ##: thief target  Giant cannot be held");
    screen.endFrame();
}

void DisplayManager::showTwoPlayerGame(const StageManager& first, const StageManager& second)
{
    screen.beginFrame();
    twoPlayerField(first, kHoldX, 1);
    twoPlayerField(second, kPanelX, 2);
    text(kHoldX, 2, ConsoleScreen::WHITE, "LOCAL 2P  /  LEVEL %d", first.getLevel() + 1);
    if (first.getPhase() == StageManager::Phase::GameOver && second.getPhase() == StageManager::Phase::GameOver) {
        text(kHoldX, 3, ConsoleScreen::YELLOW, "GAME OVER / R: restart both boards");
    }
    text(kHoldX, kFooterY, ConsoleScreen::GRAY, "%s", first.getPhase() == StageManager::Phase::Choosing
        ? "1P I/K: select  Space/S: confirm" : "1P I/J/K/L, Shift/W, Space/S");
    text(kPanelX, kFooterY, ConsoleScreen::GRAY, "%s", second.getPhase() == StageManager::Phase::Choosing
        ? "2P numpad 5/2: select  Down: confirm" : "2P numpad 5/1/2/3, Up/Down");
    screen.writeWideText(kHoldX, kFooterY + 1, ConsoleScreen::GRAY,
        L"ESC: menu  \u25A3: reinforced  ##: thief target");
    screen.endFrame();
}

void DisplayManager::showMainMenu()
{
    constexpr int x = ConsoleScreen::kFrameColumns / 2 - 19;
    screen.beginFrame();
    text(x, 8, ConsoleScreen::WHITE, "BLOCK KINGDOM REBUILD  " TETRIS_STRING_VALUE(TETRIS_VERSION));
    text(x, 11, ConsoleScreen::SKY_BLUE, "[1]  1P Story");
    text(x, 13, ConsoleScreen::GREEN, "[2]  Local 2P");
    text(x, 15, ConsoleScreen::GREEN, "[3]  1P Endless");
    text(x, 18, ConsoleScreen::GRAY, "ESC  Quit");
    screen.endFrame();
}
