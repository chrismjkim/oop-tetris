// 실제 게임/렌더러를 연결하고 Windows 콘솔 기능만 대체한다.
// 실행 시 표준 입력으로 1과 줄바꿈을 전달한다: echo 1 | game_smoke.exe
#include "../console.h"
#include "../game.h"

#include <cassert>
#include <cstdio>

namespace {
int availability_checks = 0;
int key_reads = 0;
int clears = 0;
unsigned sleeps = 0;
const int keys[] = {' ', 0, 0x4b, 224, 0x4d, 224, 0x48, 0, 0x50};
}

namespace tetris {
namespace console {
void initialize() {}
void gotoxy(int x, int y)
{
    assert(x >= 0 && x < 80);
    assert(y >= 0 && y < 25);
}
void set_color(int color) { assert(color >= BLACK && color <= WHITE); }
void sleep_ms(unsigned) { assert(++sleeps < 50000); }
void clear() { ++clears; }
bool key_available() { return availability_checks++ % 2 == 0; }
int read_key()
{
    const int index = key_reads++;
    if (index < static_cast<int>(sizeof(keys) / sizeof(keys[0]))) {
        return keys[index];
    }
    return ' ';
}
void discard_pending_keys()
{
    while (key_available()) {
        read_key();
    }
}
} // namespace console
} // namespace tetris

int main()
{
    tetris::run_game();
    // 로고 종료 -> 레벨 선택 -> 반복 하드드롭 -> 게임 오버 -> 입력 EOF 종료.
    assert(clears == 3);
    assert(key_reads > 12);
    std::puts("\nPASS: intro, level selection, arrow keys, repeated hard drop and game over");
}
