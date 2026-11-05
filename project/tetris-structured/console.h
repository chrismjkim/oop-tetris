#pragma once

namespace tetris {
namespace console {

enum Color {
    BLACK, DARK_BLUE, DARK_GREEN, DARK_SKY_BLUE, DARK_RED, DARK_VIOLET,
    DARK_YELLOW, GRAY, DARK_GRAY, BLUE, GREEN, SKY_BLUE, RED, VIOLET,
    YELLOW, WHITE
};

void initialize();
void gotoxy(int x, int y);
void set_color(int color);
void sleep_ms(unsigned milliseconds);
void clear();
bool key_available();
int read_key();
void discard_pending_keys();

} // namespace console
} // namespace tetris
