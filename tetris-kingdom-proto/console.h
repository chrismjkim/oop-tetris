#pragma once

namespace tetris {
namespace console {

constexpr int kFrameColumns = 108;
constexpr int kFrameRows = 30;

enum Color {
    BLACK, DARK_BLUE, DARK_GREEN, DARK_SKY_BLUE, DARK_RED, DARK_VIOLET,
    DARK_YELLOW, GRAY, DARK_GRAY, BLUE, GREEN, SKY_BLUE, RED, VIOLET,
    YELLOW, WHITE,
    BLOCK_I, BLOCK_O, BLOCK_T, BLOCK_J, BLOCK_L, BLOCK_Z, BLOCK_S
};

enum class LocalKey {
    None, Ignored, Escape, Restart,
    FirstRotate, FirstLeft, FirstSoftDrop, FirstRight, FirstHold, FirstHardDrop,
    SecondRotate, SecondLeft, SecondSoftDrop, SecondRight, SecondHold, SecondHardDrop
};

void initialize();
void shutdown();
void begin_frame();
void write_text(int x, int y, int color, const char* text);
constexpr int glyph_columns(wchar_t ch)
{
    return (ch >= L'\uAC00' && ch <= L'\uD7A3') || ch == L'\u25A3' ? 2 : 1;
}
void write_wide_text(int x, int y, int color, const wchar_t* text);
enum class BlockStyle { Solid, Reinforced, CompactReinforced };
void write_block(int x, int y, int color, BlockStyle style = BlockStyle::Solid);
void end_frame();
void gotoxy(int x, int y);
void set_color(int color);
void sleep_ms(unsigned milliseconds);
void clear();
bool key_available();
int read_key();
LocalKey read_local_key();
void discard_pending_keys();

} // namespace console
} // namespace tetris
