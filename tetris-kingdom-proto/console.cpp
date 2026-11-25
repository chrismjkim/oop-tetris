#include "console.h"
#include <conio.h>
#include <Windows.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <string>

namespace tetris {
namespace console {
namespace {
HANDLE original_output = INVALID_HANDLE_VALUE;
HANDLE output = INVALID_HANDLE_VALUE;
HANDLE local_input = INVALID_HANDLE_VALUE;
CHAR_INFO frame[kFrameColumns * kFrameRows];
unsigned int frame_rgb[kFrameColumns * kFrameRows];
CHAR_INFO presented_frame[kFrameColumns * kFrameRows];
unsigned int presented_rgb[kFrameColumns * kFrameRows];
CONSOLE_SCREEN_BUFFER_INFO presented_info{};
bool have_presented_frame = false;
using ChangedRows = std::bitset<kFrameRows>;
CONSOLE_CURSOR_INFO old_cursor{};
DWORD original_output_mode = 0;
bool restore_output_mode = false;
bool vt_enabled = false;
std::array<bool, 256> local_pressed{};

LocalKey local_binding(const KEY_EVENT_RECORD& key)
{
    const bool enhanced = (key.dwControlKeyState & ENHANCED_KEY) != 0;
    switch (key.wVirtualKeyCode) {
    case VK_ESCAPE: return LocalKey::Escape;
    case 'R': return LocalKey::Restart;
    case 'I': return LocalKey::FirstRotate;
    case 'J': return LocalKey::FirstLeft;
    case 'K': return LocalKey::FirstSoftDrop;
    case 'L': return LocalKey::FirstRight;
    case 'W':
    case VK_LSHIFT: return LocalKey::FirstHold;
    case VK_SHIFT: return key.wVirtualScanCode == 0x2A ? LocalKey::FirstHold : LocalKey::Ignored;
    case 'S':
    case VK_SPACE: return LocalKey::FirstHardDrop;
    case VK_NUMPAD5: return LocalKey::SecondRotate;
    case VK_NUMPAD1: return LocalKey::SecondLeft;
    case VK_NUMPAD2: return LocalKey::SecondSoftDrop;
    case VK_NUMPAD3: return LocalKey::SecondRight;
    // Num Lock이 꺼져도 같은 물리 키를 사용한다. 방향키의 확장 비트로 구분한다.
    case VK_CLEAR: return !enhanced && key.wVirtualScanCode == 0x4C ? LocalKey::SecondRotate : LocalKey::Ignored;
    case VK_END: return !enhanced && key.wVirtualScanCode == 0x4F ? LocalKey::SecondLeft : LocalKey::Ignored;
    case VK_NEXT: return !enhanced && key.wVirtualScanCode == 0x51 ? LocalKey::SecondRight : LocalKey::Ignored;
    case VK_UP: return enhanced ? LocalKey::SecondHold : LocalKey::Ignored;
    case VK_DOWN: return enhanced ? LocalKey::SecondHardDrop :
        (key.wVirtualScanCode == 0x50 ? LocalKey::SecondSoftDrop : LocalKey::Ignored);
    default: return LocalKey::Ignored;
    }
}

struct BlockPaletteEntry {
    unsigned int rgb;
    Color fallback;
};

// TETRIS 99 기본 테마 화면에서 조각 면의 중앙색을 뽑은 값이다.
constexpr BlockPaletteEntry kBlockPalette[] = {
    {0x07D5F2, SKY_BLUE},    // I
    {0xFACF21, YELLOW},      // O
    {0xA803F5, VIOLET},      // T
    {0x260BE4, BLUE},        // J
    {0xF37323, DARK_YELLOW}, // L
    {0xE92648, RED},         // Z
    {0x6CED18, GREEN}        // S
};

const BlockPaletteEntry* block_color(int color)
{
    if (color < BLOCK_I || color > BLOCK_S) return nullptr;
    return &kBlockPalette[color - BLOCK_I];
}

int ansi_color(int color, int normal, int bright)
{
    // Win32 색상 인덱스의 빨강과 파랑 순서를 ANSI에 맞춰 변환한다.
    constexpr int index[] = {0, 4, 2, 6, 1, 5, 3, 7};
    return (color & 8 ? bright : normal) + index[color & 7];
}

void append_color(std::wstring& sequence, int indexed, unsigned int rgb, bool background)
{
    if (rgb == 0) {
        sequence += std::to_wstring(ansi_color(indexed, background ? 40 : 30, background ? 100 : 90));
        return;
    }
    sequence += background ? L"48;2;" : L"38;2;";
    sequence += std::to_wstring((rgb >> 16) & 0xFF);
    sequence += L';';
    sequence += std::to_wstring((rgb >> 8) & 0xFF);
    sequence += L';';
    sequence += std::to_wstring(rgb & 0xFF);
}

bool write_vt_frame(const ChangedRows& changed, int columns, int rows)
{
    static std::wstring sequence;
    sequence.clear();
    sequence.reserve(4096);
    WORD previous_style = 0xFFFF;
    unsigned int previous_rgb = 0;
    for (int y = 0; y < rows; ++y) {
        if (!changed[y]) continue;
        sequence += L"\x1b[";
        sequence += std::to_wstring(y + 1);
        sequence += L";1H";
        for (int x = 0; x < columns; ++x) {
            const int index = y * kFrameColumns + x;
            const CHAR_INFO& cell = frame[index];
            if (cell.Attributes & COMMON_LVB_TRAILING_BYTE) continue;
            const WORD style = cell.Attributes & (0xFF | COMMON_LVB_GRID_HORIZONTAL);
            const unsigned int rgb = frame_rgb[index];
            if (style != previous_style || rgb != previous_rgb) {
                const bool filled = (style & COMMON_LVB_GRID_HORIZONTAL) != 0;
                sequence += L"\x1b[0;";
                append_color(sequence, style & 0xF, filled ? 0 : rgb, false);
                sequence += L';';
                append_color(sequence, (style >> 4) & 0xF, filled ? rgb : 0, true);
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
                sequence += std::to_wstring(x + 1);
                sequence += L"G\u25A3";
                if (x + 2 < columns) {
                    sequence += L"\x1b[";
                    sequence += std::to_wstring(x + 3);
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

bool write_native_rows(const ChangedRows& changed, const SMALL_RECT& window, int columns, int rows)
{
    for (int y = 0; y < rows;) {
        if (!changed[y]) { ++y; continue; }
        const auto first = static_cast<SHORT>(y);
        do { ++y; } while (y < rows && changed[y]);
        const SMALL_RECT band{window.Left, static_cast<SHORT>(window.Top + first),
            static_cast<SHORT>(window.Left + columns - 1), static_cast<SHORT>(window.Top + y - 1)};
        SMALL_RECT written = band;
        if (!WriteConsoleOutputW(output, frame, COORD{kFrameColumns, kFrameRows}, COORD{0, first}, &written)
            || std::memcmp(&written, &band, sizeof band) != 0) return false;
    }
    return true;
}
}

void initialize()
{
    have_presented_frame = false;
    local_pressed.fill(false);
    local_input = CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);
    original_output = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(original_output, &old_cursor);
    output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
    if (output == INVALID_HANDLE_VALUE) output = original_output;
    SMALL_RECT window = {0, 0, kFrameColumns - 1, kFrameRows - 1};
    // 새 버퍼만 조절하며 종료 시 원래 콘솔 버퍼로 돌아간다.
    if (output != original_output) {
        SetConsoleWindowInfo(output, TRUE, &window);
        SetConsoleScreenBufferSize(output, COORD{kFrameColumns, kFrameRows});
        SetConsoleWindowInfo(output, TRUE, &window);
        SetConsoleActiveScreenBuffer(output);
    }
    DWORD mode = 0;
    if (GetConsoleMode(output, &mode)) {
        const DWORD grid_mode = mode | ENABLE_LVB_GRID_WORLDWIDE;
        constexpr DWORD vt_mode = ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        vt_enabled = SetConsoleMode(output, grid_mode | vt_mode) != FALSE;
        if ((vt_enabled || SetConsoleMode(output, grid_mode)) && output == original_output) {
            original_output_mode = mode;
            restore_output_mode = true;
        }
    }
    const CONSOLE_CURSOR_INFO hidden = {25, FALSE};
    SetConsoleCursorInfo(output, &hidden);
}

void shutdown()
{
    if (local_input != INVALID_HANDLE_VALUE) {
        CloseHandle(local_input);
        local_input = INVALID_HANDLE_VALUE;
    }
    if (output != original_output && output != INVALID_HANDLE_VALUE) {
        SetConsoleActiveScreenBuffer(original_output);
        CloseHandle(output);
    } else {
        if (restore_output_mode) SetConsoleMode(original_output, original_output_mode);
        SetConsoleCursorInfo(original_output, &old_cursor);
    }
    restore_output_mode = false;
    vt_enabled = false;
    output = INVALID_HANDLE_VALUE;
}

void begin_frame()
{
    for (int index = 0; index < kFrameColumns * kFrameRows; ++index) {
        frame[index].Char.UnicodeChar = L' ';
        frame[index].Attributes = GRAY;
        frame_rgb[index] = 0;
    }
}

void write_text(int x, int y, int color, const char* value)
{
    if (y < 0 || y >= kFrameRows) return;
    const BlockPaletteEntry* block = block_color(color);
    for (int offset = 0; value[offset] != '\0' && x + offset < kFrameColumns; ++offset) {
        if (x + offset < 0) continue;
        const int index = y * kFrameColumns + x + offset;
        CHAR_INFO& cell = frame[index];
        cell.Char.UnicodeChar = static_cast<unsigned char>(value[offset]);
        cell.Attributes = static_cast<WORD>(block ? block->fallback : color);
        frame_rgb[index] = block ? block->rgb : 0;
    }
}

void write_wide_text(int x, int y, int color, const wchar_t* value)
{
    if (x < 0 || y < 0 || y >= kFrameRows) return;
    for (; *value; ++value) {
        const int width = glyph_columns(*value);
        if (x + width > kFrameColumns) break;
        const int index = y * kFrameColumns + x;
        CHAR_INFO& cell = frame[index];
        cell.Char.UnicodeChar = *value;
        cell.Attributes = static_cast<WORD>(color);
        frame_rgb[index] = 0;
        if (width == 2) {
            frame[index + 1] = cell;
            cell.Attributes |= COMMON_LVB_LEADING_BYTE;
            frame[index + 1].Attributes |= COMMON_LVB_TRAILING_BYTE;
            frame_rgb[index + 1] = 0;
        }
        x += width;
    }
}

void write_block(int x, int y, int color, BlockStyle style)
{
    const int width = style == BlockStyle::CompactReinforced ? 1 : 2;
    if (x < 0 || x + width > kFrameColumns || y < 0 || y >= kFrameRows) return;
    const BlockPaletteEntry* block = block_color(color);
    const int indexed = block ? block->fallback : color;
    const int index = y * kFrameColumns + x;
    const WORD face = static_cast<WORD>((indexed << 4) | COMMON_LVB_GRID_HORIZONTAL);
    CHAR_INFO& left = frame[index];
    left.Char.UnicodeChar = L' ';
    const bool light_mark = style != BlockStyle::Solid && (color == BLOCK_J || color == BLOCK_T);
    left.Attributes = static_cast<WORD>(face | (light_mark ? WHITE : BLACK));
    frame_rgb[index] = block ? block->rgb : 0;
    if (style == BlockStyle::CompactReinforced) {
        left.Char.UnicodeChar = L'\u29C8'; // ⧈는 축소 미리보기 한 문자 칸에 들어간다.
        return;
    }
    CHAR_INFO& right = frame[index + 1];
    right = left;
    frame_rgb[index + 1] = frame_rgb[index];
    if (style == BlockStyle::Reinforced) {
        left.Char.UnicodeChar = right.Char.UnicodeChar = L'\u25A3';
        left.Attributes |= COMMON_LVB_LEADING_BYTE;
        right.Attributes |= COMMON_LVB_TRAILING_BYTE;
    } else {
        right.Attributes |= COMMON_LVB_GRID_RVERTICAL;
    }
}

void end_frame()
{
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(output, &info)) {
        have_presented_frame = false;
        SMALL_RECT region{0, 0, kFrameColumns - 1, kFrameRows - 1};
        WriteConsoleOutputW(output, frame, COORD{kFrameColumns, kFrameRows}, COORD{0, 0}, &region);
        return;
    }
    const auto& window = info.srWindow;
    const int columns = (std::min)(kFrameColumns, window.Right - window.Left + 1);
    const int rows = (std::min)(kFrameRows, window.Bottom - window.Top + 1);
    const bool full = !have_presented_frame || info.dwSize.X != presented_info.dwSize.X
        || info.dwSize.Y != presented_info.dwSize.Y
        || std::memcmp(&window, &presented_info.srWindow, sizeof window) != 0;
    ChangedRows changed;
    for (int y = 0; y < rows; ++y) {
        const int start = y * kFrameColumns;
        changed[y] = full || std::memcmp(frame + start, presented_frame + start, columns * sizeof frame[0]) != 0
            || std::memcmp(frame_rgb + start, presented_rgb + start, columns * sizeof frame_rgb[0]) != 0;
    }
    if (changed.none()) return;
    if (vt_enabled) {
        if (!write_vt_frame(changed, columns, rows)) {
            // 부분 출력으로 깨진 행과 색을 복구하고, 다음 프레임에 VT 출력을 다시 시도한다.
            have_presented_frame = false;
            changed.set();
            write_native_rows(changed, window, columns, rows);
            return;
        }
    } else if (!write_native_rows(changed, window, columns, rows)) {
        have_presented_frame = false;
        return;
    }
    std::memcpy(presented_frame, frame, sizeof frame);
    std::memcpy(presented_rgb, frame_rgb, sizeof frame_rgb);
    presented_info = info;
    have_presented_frame = true;
}

void gotoxy(int x, int y)
{
    SetConsoleCursorPosition(output, COORD{static_cast<SHORT>(x), static_cast<SHORT>(y)});
}
void set_color(int color) { SetConsoleTextAttribute(output, static_cast<WORD>(color)); }
void sleep_ms(unsigned milliseconds) { Sleep(milliseconds); }
void clear() { begin_frame(); end_frame(); }
bool key_available() { return _kbhit() != 0; }
int read_key() { return _getch(); }
LocalKey read_local_key()
{
    DWORD available = 0;
    if (!GetNumberOfConsoleInputEvents(local_input, &available) || available == 0) return LocalKey::None;

    INPUT_RECORD record{};
    DWORD read = 0;
    if (!ReadConsoleInputW(local_input, &record, 1, &read) || read == 0) return LocalKey::None;
    if (record.EventType == FOCUS_EVENT && !record.Event.FocusEvent.bSetFocus) local_pressed.fill(false);
    if (record.EventType != KEY_EVENT) return LocalKey::Ignored;

    const KEY_EVENT_RECORD& key = record.Event.KeyEvent;
    const LocalKey action = local_binding(key);
    if (action == LocalKey::Ignored) return action;
    const auto vk = key.wVirtualKeyCode;
    const bool pressed_before = local_pressed[vk];
    local_pressed[vk] = key.bKeyDown != FALSE;
    if (!key.bKeyDown) return LocalKey::Ignored;
    const bool one_shot = action == LocalKey::FirstHold || action == LocalKey::FirstHardDrop ||
        action == LocalKey::SecondHold || action == LocalKey::SecondHardDrop || action == LocalKey::Restart;
    return one_shot && pressed_before ? LocalKey::Ignored : action;
}
void discard_pending_keys()
{
    local_pressed.fill(false);
    while (key_available()) read_key();
}

} // namespace console
} // namespace tetris
