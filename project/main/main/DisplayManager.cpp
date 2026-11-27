#include "DisplayManager.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <conio.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::wstring;
using std::to_wstring;

static constexpr Colors blockColors[] = {
	Colors::DARK_BLUE, Colors::DARK_GREEN, Colors::DARK_SKY_BLUE,
	Colors::DARK_RED, Colors::DARK_VOILET, Colors::DARK_YELLOW
};

static wstring rgbToWstring(int color) {
	// int를 rgb string으로 변환
	const int red = (color >> 16) & 0xFF;
	const int green = (color >> 8) & 0xFF;
	const int blue = color & 0xFF;
	return to_wstring(red) + L";" + to_wstring(green) + L";" + to_wstring(blue);
}

static void writeConsoleText(const wstring& text) {
	DWORD written;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(),
		static_cast<DWORD>(text.size()), &written, nullptr);
}

static void writeCell(const wstring& background) {
	// 검정 글자색, 셀 배경색, 윗줄과 오른쪽 구분선
	writeConsoleText(L"\x1b[0;30;" + background + L";53m \u2595\x1b[0m");
}

DisplayManager::DisplayManager() {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE; 
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	// 블록 구분선을 그리기 위한 설정
	DWORD mode; //unsigned long
	if (GetConsoleMode(hConsole, &mode)) {
		SetConsoleMode(hConsole, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
}

void DisplayManager::setTextColor(Colors color) {

	const int icolor = static_cast<int>(color);
	writeConsoleText(L"\x1b[38;2;" + rgbToWstring(icolor) + L"m");
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
	for (int r = 0; r <= ROW; r++) {
		gotoxy((-1 * WIDTH) + ab_x + 2, r + ab_y);
		drawCell(Colors::DARK_GRAY);

		gotoxy((COL * WIDTH) + ab_x + 2, r + ab_y);
		drawCell(Colors::DARK_GRAY);
	}

	for (int c = -1; c <= COL; c++) {
		gotoxy((c * WIDTH) + ab_x + 2, ROW + ab_y);
		drawCell(Colors::DARK_GRAY);
	}
}

void DisplayManager::showLayout() {
	showHold();
	showGameBoard();
	showNext();
	showStageArt();
	showControls();
}

void DisplayManager::showHUD(int level, int score, int lines) {
	setTextColor(Colors::WHITE);
	gotoxy(1, 0);
	writeConsoleText(L"SCORE " + to_wstring(score) + L"          ");
	gotoxy(22, 0);
	writeConsoleText(L"LINES " + to_wstring(lines) + L"          ");
	gotoxy(42, 0);
	writeConsoleText(L"STAGE " + to_wstring(level) + L" ");
}

void DisplayManager::showPreviewFrame(int x, const wchar_t* title) {
	setTextColor(Colors::GRAY);
	gotoxy(x, ab_y);
	writeConsoleText(wstring(L"+- ") + title + L" - -+");
	for (int row = 1; row <= 5; ++row) {
		gotoxy(x, ab_y + row);
		writeConsoleText(L"|          |");
	}
	gotoxy(x, ab_y + 6);
	writeConsoleText(L"+----------+");
}

void DisplayManager::showHold() {
	showPreviewFrame(ab_x - previewWidth, L"HOLD");
}

void DisplayManager::showNext() {
	showPreviewFrame(ab_x + (COL + 2) * WIDTH, L"NEXT");
}

void DisplayManager::showStageArt() {
	const int x = ab_x + (COL + 2) * WIDTH + previewWidth + 6;
	const int artWidth = 52;
	for (int row = 0; row < ROW; ++row) {
		gotoxy(x, ab_y + row);
		drawCell(Colors::DARK_GRAY);
		gotoxy(x + artWidth - WIDTH, ab_y + row);
		drawCell(Colors::DARK_GRAY);
	}
	for (int col = 0; col < artWidth; col += WIDTH) {
		gotoxy(x + col, ab_y + ROW);
		drawCell(Colors::DARK_GRAY);
	}
}

void DisplayManager::showControls() {
	setTextColor(Colors::GRAY);
	gotoxy(ab_x - previewWidth, ab_y + ROW + 1);
	writeConsoleText(L"Arrows: move/rotate/drop  SPACE: hard drop  ESC: quit");
}

int DisplayManager::showKeys() {
	int in = 0;
	setTextColor(Colors::GRAY);
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
	showHUD(level, score, lines);
}



void DisplayManager::showNextBlockContainer(BlockType type, int level) {
	int i, j;
	const Colors color = blockColors[(static_cast<unsigned>(level) + 1) % 6];
	for (i = 1;i < 7;i++)
	{
		gotoxy(33, i);
		for (j = 0;j < 6;j++)
		{
			if (i == 1 || i == 6 || j == 0 || j == 5)
			{
				drawCell(color);
			}
			else {
				writeConsoleText(L"  ");
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

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			gotoxyFromRC(r + i, c + j);
			if (cells[i][j] != nullptr) {
				drawCell(cells[i][j]->getColor());
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

				drawCell(cell->getColor());
			}
			else {
				gotoxyFromRC(r, c);

				setTextColor(Colors::DARK_GRAY);
				cout << " .";
			}
		}
	}
}

void DisplayManager::drawCell(int color) {
	// cell의 경우 배경색을 변경
	writeConsoleText(L"\x1b[0;30;48;2;" + rgbToWstring(color) + L";53m \u2595\x1b[0m");
}

void DisplayManager::drawCell(Colors color) {
	drawCell(static_cast<int>(color));
}
