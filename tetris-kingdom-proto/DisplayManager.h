#pragma once

#include "ConsoleScreen.h"
#include "StageManager.h"

class DisplayManager {
public:
    void showGame(const StageManager& game, bool help = false);
    void showTwoPlayerGame(const StageManager& first, const StageManager& second);
    void showMainMenu();

private:
    ConsoleScreen screen;
    static constexpr int kHoldX = 1;
    static constexpr int kBoardY = 7;
    static constexpr int kBoardWidth = (Board::boardWidth + 2) * 2;
    static constexpr int kBoardRows = Board::height + 1;
    static constexpr int kPreviewWidth = 12;
    static constexpr int kPanelX = 55;
    static constexpr int kArtWidth = 52;
    static constexpr int kFooterY = kBoardY + kBoardRows;
    static constexpr int kWallColor = ConsoleScreen::DARK_GRAY;
    void text(int x, int y, int color, const char* format, ...);
    void drawActive(const Block& piece, int x, bool ghost);
    void preview(int x, const char* name, const BlockSpec* spec);
    void board(const StageManager& game, int x);
    static const char* thiefStatus(const StageManager& game);
    void field(const StageManager& game, int x);
    void construction(const StageManager& game);
    void twoPlayerField(const StageManager& game, int x, int player);
};
