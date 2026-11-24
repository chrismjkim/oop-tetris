#pragma once

#include <array>

#include "BlockType.h"
#include "Block.h"

using std::array;

enum {
	BLACK,      /*  0 : 까망 */
	DARK_BLUE,    /*  1 : 어두운 파랑 */
	DARK_GREEN,    /*  2 : 어두운 초록 */
	DARK_SKY_BLUE,  /*  3 : 어두운 하늘 */
	DARK_RED,    /*  4 : 어두운 빨강 */
	DARK_VOILET,  /*  5 : 어두운 보라 */
	DARK_YELLOW,  /*  6 : 어두운 노랑 */
	GRAY,      /*  7 : 회색 */
	DARK_GRAY,    /*  8 : 어두운 회색 */
	BLUE,      /*  9 : 파랑 */
	GREEN,      /* 10 : 초록 */
	SKY_BLUE,    /* 11 : 하늘 */
	RED,      /* 12 : 빨강 */
	VOILET,      /* 13 : 보라 */
	YELLOW,      /* 14 : 노랑 */
	WHITE
};

// 화면 출력 및 연출 담당
class DisplayManager
{
private:
	static inline constexpr int ROW = 21;
	static inline constexpr int COL = 12;
	const int WIDTH = 2;
	int ab_x = 5;
	int ab_y = 1;
	bool cursorWasVisible = true;
public:
	DisplayManager();
	~DisplayManager();
	void gotoxy(int x, int y);
	void showLogo();
	int showKeys();
	void showBlock(Block& block);
	void showGameBoard(int stage, array<array<int, 10>, 20> cellList);
	void showGameStat(const int level, const int score, const int lines);
	void setColor(int color);
	void resetab_xy();
	void showNextBlockContainer(BlockType type, int level);
};

