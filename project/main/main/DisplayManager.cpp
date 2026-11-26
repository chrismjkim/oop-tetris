#include "DisplayManager.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <conio.h>


using namespace std;

DisplayManager::DisplayManager() {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE; 
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void DisplayManager::setColor(int color) {
	static HANDLE std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, color);
}

void DisplayManager::setColor(Colors color) {
	int icolor = static_cast<int>(color);
	static HANDLE std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, icolor);
}

void DisplayManager::gotoxy(int x, int y) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.Y = y;
	pos.X = x;
	SetConsoleCursorPosition(hConsole, pos);

}

void DisplayManager::gotoxyFromRC(int row, int col) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.Y = row + ab_y;
	pos.X = (col * WIDTH) + ab_x + 2;
	// +2 는 좌측 벽면 사이즈
	SetConsoleCursorPosition(hConsole, pos);
}


void DisplayManager::resetab_xy() {
	ab_x = Util::getDisplayX();
	ab_y = Util::getDisplayY();
}

void DisplayManager::showLogo() {
	const string logo[] = {
	"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓",
	"┃  ◆◆◆   ◆◆◆   ◆◆◆    ◆◆     ◆   ◆◆◆           ┃",
	"┃   ◆    ◆      ◆     ◆ ◆    ◆   ◆             ┃",
	"┃   ◆    ◆◆◆    ◆     ◆◆     ◆     ◆           ┃",
	"┃   ◆    ◆      ◆     ◆ ◆    ◆      ◆          ┃",
	"┃   ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆           ┃",
	"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\0"
	};

	for (int i = 0; i < 7; i++) {
		gotoxy(13, 3 + i);
		cout << logo[i] << endl;
		Sleep(100);
	}
	gotoxy(28, 20);
	cout << "Please Press Any Key." << endl;

	while (true) {
		if (_kbhit())
			break;
		Sleep(30);

	}

	_getch();
	system("cls");


}

void DisplayManager::showGameBoard() {
	setColor(Colors::DARK_GRAY);

	for (int r = 0; r <= ROW; r++) {
		gotoxy((-1 * WIDTH) + ab_x + 2, r + ab_y);
		cout << "■";

		gotoxy((COL * WIDTH) + ab_x + 2, r + ab_y);
		cout << "■";
	}

	for (int c = -1; c <= COL; c++) {
		gotoxy((c * WIDTH) + ab_x + 2, ROW + ab_y);
		cout << "■";
	}
}
int DisplayManager::showKeys() {
	int in = 0;
	setColor(Colors::GRAY);
	const string ip[] = {
		"┏━━━━━━━━━<GAME KEY>━━━━━━━━━┓",
		"┃ UP   : Rotate Block        ┃",
		"┃ DOWN : Move One-Step Down  ┃",
		"┃ SPACE: Move Bottom Down    ┃",
		"┃ LEFT : Move Left           ┃",
		"┃ RIGHT: Move Right          ┃",
		"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛"
	};

	for (int i = 0; i < 7; i++) {
		gotoxy(10, 7 + i);
		cout << ip[i] << endl;
		Sleep(10);
	}
	while (in < 1 || in>8) {
		gotoxy(10, 3);
		cout << "Select Start level[1-8]:       \b\b\b\b\b\b\b";
		cin >> in;
	}
	system("cls");
	return in;
}

void DisplayManager::showGameStat(const int level, const int score, const int lines) {
	static int printed_text = 0;
	setColor(Colors::GRAY);
	if (printed_text == 0)
	{
		gotoxy(35, 7);
		printf("STAGE");
		gotoxy(35, 9);
		printf("SCORE");
		gotoxy(35, 12);
		printf("LINES");
	}
	gotoxy(41, 7);
	printf("%d", level);
	gotoxy(35, 10);
	printf("%10d", score);
}



void DisplayManager::showNextBlockContainer(BlockType type, int level) {
	int i, j;
	setColor((level + 1) % 6 + 1);
	for (i = 1;i < 7;i++)
	{
		gotoxy(33, i);
		for (j = 0;j < 6;j++)
		{
			if (i == 1 || i == 6 || j == 0 || j == 5)
			{
				printf("■");
			}
			else {
				printf("  ");
			}

		}
	}
}

void DisplayManager::eraseBlock(Block& block) {
	int r = block.getRow();
	int c = block.getCol();	
	auto cells = block.getCells();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i + r < Util::MAXBOARDROW && j + c < Util::MAXBOARDCOL) {
				gotoxyFromRC(r + i, c + j);
				if (cells[i][j] != nullptr) {
					cout << "";
				}
			}
		}
	}
}

void DisplayManager::showBlock(Block& block) {
	int r = block.getRow();
	int c = block.getCol();
	auto cells = block.getCells();

	setColor(static_cast<int>(block.getType()) % 6 + 1);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			gotoxyFromRC(r + i, c + j);
			if (cells[i][j] != nullptr) {
				cout << "■";
			}
		}
	}
}

void DisplayManager::showCells(array<array<Cell*,Util::MAXBOARDCOL>, Util::MAXBOARDROW> cellList) {

	for (int r = 0; r < Util::MAXBOARDROW; r++) {
		for (int c = 0; c < Util::MAXBOARDCOL; c++) {
			auto cell = cellList[r][c];
			if (cell != nullptr) {
				gotoxyFromRC(r, c); 
				setColor(cell->getColor());
				cout << "■";

			}
			else {
				gotoxyFromRC(r, c);
				setColor(0);
				cout << "  ";
			}
		}
	}

	
}
