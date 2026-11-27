#pragma once

#include <array>

#include "Util.h"
#include "BlockType.h"
#include "Block.h"
#include "Colors.h"

using std::array;


// 화면 출력 및 연출 담당
class DisplayManager
{
private:
	static inline constexpr int ROW = Util::MAXBOARDROW;
	static inline constexpr int COL = Util::MAXBOARDCOL;
	const int WIDTH = 2;
	int ab_x = Util::getDisplayX();
	int ab_y = Util::getDisplayY();
public:
	DisplayManager();
	void gotoxy(int x, int y);
	void gotoxyFromRC(int row, int col);
	void showLogo();
	int showKeys();
	void showBlock(Block& block);
	void showGameBoard();
	void showGameStat(const int level, const int score, const int lines);
	void setColor(int color);
	void setColor(Colors color);
	void resetab_xy();
	void showNextBlockContainer(BlockType type, int level);
	void eraseBlock(Block& block);
	void showCells(array<array<Cell*, Util::MAXBOARDCOL>, Util::MAXBOARDROW> cellList);
};

