#include "ConsoleScreen.h"
#include <algorithm>
#include <cstring>

using namespace std;

ConsoleScreen::ConsoleScreen()
{
    havePresentedFrame = false;
    originalOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(originalOutput, &oldCursor);
    output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
    if (output == INVALID_HANDLE_VALUE) output = originalOutput;
    else screenBuffer.reset(output);
    SMALL_RECT window = {0, 0, kFrameColumns - 1, kFrameRows - 1};
    // 새 버퍼만 조절하며 종료 시 원래 콘솔 버퍼로 돌아간다.
    if (output != originalOutput) {
        SetConsoleWindowInfo(output, TRUE, &window);
        SetConsoleScreenBufferSize(output, COORD{kFrameColumns, kFrameRows});
        SetConsoleWindowInfo(output, TRUE, &window);
        SetConsoleActiveScreenBuffer(output);
    }
    DWORD mode = 0;
    if (GetConsoleMode(output, &mode)) {
        const DWORD grid_mode = mode | ENABLE_LVB_GRID_WORLDWIDE;
        constexpr DWORD vt_mode = ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        vtEnabled = SetConsoleMode(output, grid_mode | vt_mode) != FALSE;
        if ((vtEnabled || SetConsoleMode(output, grid_mode)) && output == originalOutput) {
            originalOutputMode = mode;
            restoreOutputMode = true;
        }
    }
    const CONSOLE_CURSOR_INFO hidden = {25, FALSE};
    SetConsoleCursorInfo(output, &hidden);
}

ConsoleScreen::~ConsoleScreen()
{
    if (screenBuffer) {
        SetConsoleActiveScreenBuffer(originalOutput);
    } else {
        if (restoreOutputMode) SetConsoleMode(originalOutput, originalOutputMode);
        SetConsoleCursorInfo(originalOutput, &oldCursor);
    }
}

int ConsoleScreen::glyphColumns(wchar_t ch)
{
    return (ch >= L'\uAC00' && ch <= L'\uD7A3') || ch == L'\u25A3' ? 2 : 1;
}

int ConsoleScreen::fallbackColor(int color)
{
    static constexpr Color palette[] = {SKY_BLUE, YELLOW, VIOLET, BLUE, DARK_YELLOW, RED, GREEN};
    return color >= BLOCK_I && color <= BLOCK_S ? palette[color - BLOCK_I] : color;
}

unsigned int ConsoleScreen::rgbColor(int color)
{
    static constexpr unsigned int palette[] = {0x07D5F2, 0xFACF21, 0xA803F5, 0x260BE4, 0xF37323, 0xE92648, 0x6CED18};
    return color >= BLOCK_I && color <= BLOCK_S ? palette[color - BLOCK_I] : 0;
}

int ConsoleScreen::ansiColor(int color, int normal, int bright)
{
    // Win32 색상 인덱스의 빨강과 파랑 순서를 ANSI에 맞춰 변환한다.
    constexpr int index[] = {0, 4, 2, 6, 1, 5, 3, 7};
    return (color & 8 ? bright : normal) + index[color & 7];
}

void ConsoleScreen::appendColor(wstring& sequence, int indexed, unsigned int rgb, bool background)
{
    if (rgb == 0) {
        sequence += to_wstring(ansiColor(indexed, background ? 40 : 30, background ? 100 : 90));
        return;
    }
    sequence += background ? L"48;2;" : L"38;2;";
    sequence += to_wstring((rgb >> 16) & 0xFF);
    sequence += L';';
    sequence += to_wstring((rgb >> 8) & 0xFF);
    sequence += L';';
    sequence += to_wstring(rgb & 0xFF);
}

bool ConsoleScreen::writeVtFrame(const ChangedRows& changed, int columns, int rows)
{
    sequence.clear();
    sequence.reserve(4096);
    WORD previous_style = 0xFFFF;
    unsigned int previous_rgb = 0;
    for (int y = 0; y < rows; ++y) {
        if (!changed[y]) continue;
        sequence += L"\x1b[";
        sequence += to_wstring(y + 1);
        sequence += L";1H";
        for (int x = 0; x < columns; ++x) {
            const int index = y * kFrameColumns + x;
            const CHAR_INFO& cell = frame[index];
            if (cell.Attributes & COMMON_LVB_TRAILING_BYTE) continue;
            const WORD style = cell.Attributes & (0xFF | COMMON_LVB_GRID_HORIZONTAL);
            const unsigned int rgb = frameRgb[index];
            if (style != previous_style || rgb != previous_rgb) {
                const bool filled = (style & COMMON_LVB_GRID_HORIZONTAL) != 0;
                sequence += L"\x1b[0;";
                appendColor(sequence, style & 0xF, filled ? 0 : rgb, false);
                sequence += L';';
                appendColor(sequence, (style >> 4) & 0xF, filled ? rgb : 0, true);
                if (filled) sequence += L";53";
                sequence += L'm';
                previous_style = style;
                previous_rgb = rgb;
            }
            // 오른쪽 문자 끝의 1/8만 보드 배경색으로 덮는다.
            if ((cell.Attributes & COMMON_LVB_LEADING_BYTE) && x + 1 >= columns) {
                sequence += L' ';
            } else if (cell.Char.UnicodeChar == L'\u25A3') {
                // ▣의 실제 폭은 터미널에 따라 다르다. 두 칸을 칠하고 다음 칸의 위치를 고정한다.
                sequence += L"  \x1b[";
                sequence += to_wstring(x + 1);
                sequence += L"G\u25A3";
                if (x + 2 < columns) {
                    sequence += L"\x1b[";
                    sequence += to_wstring(x + 3);
                    sequence += L'G';
                }
            } else {
                sequence += (cell.Attributes & COMMON_LVB_GRID_RVERTICAL) ? L'\u2595' : cell.Char.UnicodeChar;
            }
        }
    }
    sequence += L"\x1b[0m";
    const DWORD length = static_cast<DWORD>(sequence.size());
    DWORD written = 0;
    return WriteConsoleW(output, sequence.c_str(), length, &written, nullptr) && written == length;
}

bool ConsoleScreen::writeNativeRows(const ChangedRows& changed, const SMALL_RECT& window, int columns, int rows)
{
    for (int y = 0; y < rows;) {
        if (!changed[y]) { ++y; continue; }
        const auto first = static_cast<SHORT>(y);
        do { ++y; } while (y < rows && changed[y]);
        const SMALL_RECT band{window.Left, static_cast<SHORT>(window.Top + first),
            static_cast<SHORT>(window.Left + columns - 1), static_cast<SHORT>(window.Top + y - 1)};
        SMALL_RECT written = band;
        if (!WriteConsoleOutputW(output, frame.data(), COORD{kFrameColumns, kFrameRows}, COORD{0, first}, &written)
            || memcmp(&written, &band, sizeof band) != 0) return false;
    }
    return true;
}

void ConsoleScreen::beginFrame()
{
    for (int index = 0; index < kFrameColumns * kFrameRows; ++index) {
        frame[index].Char.UnicodeChar = L' ';
        frame[index].Attributes = GRAY;
        frameRgb[index] = 0;
    }
}

void ConsoleScreen::writeText(int x, int y, int color, const char* value)
{
    if (y < 0 || y >= kFrameRows) return;
    for (int offset = 0; value[offset] != '\0' && x + offset < kFrameColumns; ++offset) {
        if (x + offset < 0) continue;
        const int index = y * kFrameColumns + x + offset;
        CHAR_INFO& cell = frame[index];
        cell.Char.UnicodeChar = static_cast<unsigned char>(value[offset]);
        cell.Attributes = static_cast<WORD>(fallbackColor(color));
        frameRgb[index] = rgbColor(color);
    }
}

void ConsoleScreen::writeWideText(int x, int y, int color, const wchar_t* value)
{
    if (x < 0 || y < 0 || y >= kFrameRows) return;
    for (; *value; ++value) {
        const int width = glyphColumns(*value);
        if (x + width > kFrameColumns) break;
        const int index = y * kFrameColumns + x;
        CHAR_INFO& cell = frame[index];
        cell.Char.UnicodeChar = *value;
        cell.Attributes = static_cast<WORD>(color);
        frameRgb[index] = 0;
        if (width == 2) {
            frame[index + 1] = cell;
            cell.Attributes |= COMMON_LVB_LEADING_BYTE;
            frame[index + 1].Attributes |= COMMON_LVB_TRAILING_BYTE;
            frameRgb[index + 1] = 0;
        }
        x += width;
    }
}

void ConsoleScreen::writeBlock(int x, int y, int color, BlockStyle style)
{
    const int width = style == BlockStyle::CompactReinforced ? 1 : 2;
    if (x < 0 || x + width > kFrameColumns || y < 0 || y >= kFrameRows) return;
    const int indexed = fallbackColor(color);
    const int index = y * kFrameColumns + x;
    const WORD face = static_cast<WORD>((indexed << 4) | COMMON_LVB_GRID_HORIZONTAL);
    CHAR_INFO& left = frame[index];
    left.Char.UnicodeChar = L' ';
    const bool light_mark = style != BlockStyle::Solid && (color == BLOCK_J || color == BLOCK_T);
    left.Attributes = static_cast<WORD>(face | (light_mark ? WHITE : BLACK));
    frameRgb[index] = rgbColor(color);
    if (style == BlockStyle::CompactReinforced) {
        left.Char.UnicodeChar = L'\u29C8'; // ⧈는 축소 미리보기 한 문자 칸에 들어간다.
        return;
    }
    CHAR_INFO& right = frame[index + 1];
    right = left;
    frameRgb[index + 1] = frameRgb[index];
    if (style == BlockStyle::Reinforced) {
        left.Char.UnicodeChar = right.Char.UnicodeChar = L'\u25A3';
        left.Attributes |= COMMON_LVB_LEADING_BYTE;
        right.Attributes |= COMMON_LVB_TRAILING_BYTE;
    } else {
        right.Attributes |= COMMON_LVB_GRID_RVERTICAL;
    }
}

void ConsoleScreen::endFrame()
{
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(output, &info)) {
        havePresentedFrame = false;
        SMALL_RECT region{0, 0, kFrameColumns - 1, kFrameRows - 1};
        WriteConsoleOutputW(output, frame.data(), COORD{kFrameColumns, kFrameRows}, COORD{0, 0}, &region);
        return;
    }
    const auto& window = info.srWindow;
    const int columns = (min)(kFrameColumns, window.Right - window.Left + 1);
    const int rows = (min)(kFrameRows, window.Bottom - window.Top + 1);
    const bool full = !havePresentedFrame || info.dwSize.X != presentedInfo.dwSize.X
        || info.dwSize.Y != presentedInfo.dwSize.Y
        || memcmp(&window, &presentedInfo.srWindow, sizeof window) != 0;
    ChangedRows changed;
    for (int y = 0; y < rows; ++y) {
        const int start = y * kFrameColumns;
        changed[y] = full || memcmp(frame.data() + start, presentedFrame.data() + start, columns * sizeof frame[0]) != 0
            || memcmp(frameRgb.data() + start, presentedRgb.data() + start, columns * sizeof frameRgb[0]) != 0;
    }
    if (changed.none()) return;
    if (vtEnabled) {
        if (!writeVtFrame(changed, columns, rows)) {
            // 부분 출력으로 깨진 행과 색을 복구하고, 다음 프레임에 VT 출력을 다시 시도한다.
            havePresentedFrame = false;
            changed.set();
            writeNativeRows(changed, window, columns, rows);
            return;
        }
    } else if (!writeNativeRows(changed, window, columns, rows)) {
        havePresentedFrame = false;
        return;
    }
    presentedFrame = frame;
    presentedRgb = frameRgb;
    presentedInfo = info;
    havePresentedFrame = true;
}
