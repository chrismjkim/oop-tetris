#include "console.h"

#include <conio.h>
#include <cstdlib>
#include <Windows.h>

namespace tetris {
namespace console {

void initialize()
{
    // 소스와 실행 문자열은 UTF-8로 통일한다(/utf-8로 빌드).
    SetConsoleOutputCP(CP_UTF8);
}

void gotoxy(int x, int y)
{
    const COORD position = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
}

void set_color(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
}

void sleep_ms(unsigned milliseconds)
{
    Sleep(milliseconds);
}

void clear()
{
    std::system("cls");
}

bool key_available()
{
    return _kbhit() != 0;
}

int read_key()
{
    // int로 받아 확장키 접두사 0/224를 char 부호 설정과 무관하게 처리한다.
    return _getch();
}

void discard_pending_keys()
{
    while (key_available()) {
        read_key();
    }
}

} // namespace console
} // namespace tetris
