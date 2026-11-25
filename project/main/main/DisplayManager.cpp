#include "DisplayManager.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include <conio.h>


using namespace std;

DisplayManager::DisplayManager() {}

void DisplayManager::setColor(int color) {
    static HANDLE std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(std_output_handle, color);
}

void DisplayManager::gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.Y = y;
    pos.X = x;
    SetConsoleCursorPosition(hConsole, pos);

}

void DisplayManager::showBlock(Block& block) {
    int x = block.getX();
    int y = block.getY();
    auto cells = block.getFill();

    setColor(static_cast<int>(block.getType()) % 6 + 1);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++) {
            gotoxy(x + i * WIDTH, y + j);
            if (cells[i][j] == 1) {
                cout << "■";
            }
        }
    }
}

void DisplayManager::resetab_xy() {
	ab_x = 5;
	ab_y = 1;
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

void DisplayManager::showGameBoard(int stage, array<array<int, 10>, 20> cellList) {
    int i, j;
    setColor(DARK_GRAY);

    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            gotoxy((j * WIDTH) + ab_x, i + ab_y);
            if (j == 0 || i == ROW - 1 || j == COL - 1) {
                cout << "■";
            }
        }
    }

}

int DisplayManager::showKeys() {
    int in = 0;
    setColor(GRAY);
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
    setColor(GRAY);
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
    int x = block.getX();
    int y = block.getY();
    auto cells = block.getFill();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            gotoxy(i + x, j + y);
            if (cells[i][j] == 1) {
                cout << "";
            }
        }
    }
}