#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <array>
#include <memory>
#include <bitset>
#include <string>

using namespace std;

class ConsoleScreen {
public:
    enum Color {
        BLACK, DARK_BLUE, DARK_GREEN, DARK_SKY_BLUE, DARK_RED, DARK_VIOLET,
        DARK_YELLOW, GRAY, DARK_GRAY, BLUE, GREEN, SKY_BLUE, RED, VIOLET,
        YELLOW, WHITE,
        BLOCK_I, BLOCK_O, BLOCK_T, BLOCK_J, BLOCK_L, BLOCK_Z, BLOCK_S
    };

    enum class BlockStyle { Solid, Reinforced, CompactReinforced };
    static constexpr int kFrameColumns = 108;
    static constexpr int kFrameRows = 30;
    ConsoleScreen();
    ~ConsoleScreen();
    ConsoleScreen(const ConsoleScreen&) = delete;
    ConsoleScreen& operator=(const ConsoleScreen&) = delete;
    void beginFrame();
    void endFrame();
    void writeText(int x, int y, int color, const char* value);
    void writeWideText(int x, int y, int color, const wchar_t* value);
    void writeBlock(int x, int y, int color, BlockStyle style = BlockStyle::Solid);
    static int glyphColumns(wchar_t ch);

private:
    using ChangedRows = bitset<kFrameRows>;
    static int fallbackColor(int color);
    static unsigned int rgbColor(int color);
    static int ansiColor(int color, int normal, int bright);
    static void appendColor(wstring& sequence, int indexed, unsigned int rgb, bool background);
    bool writeVtFrame(const ChangedRows& changed, int columns, int rows);
    bool writeNativeRows(const ChangedRows& changed, const SMALL_RECT& window, int columns, int rows);
    unique_ptr<void, decltype(&CloseHandle)> screenBuffer{nullptr, &CloseHandle};
    HANDLE originalOutput = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    array<CHAR_INFO, kFrameColumns * kFrameRows> frame{};
    array<unsigned int, kFrameColumns * kFrameRows> frameRgb{};
    array<CHAR_INFO, kFrameColumns * kFrameRows> presentedFrame{};
    array<unsigned int, kFrameColumns * kFrameRows> presentedRgb{};
    CONSOLE_SCREEN_BUFFER_INFO presentedInfo{};
    bool havePresentedFrame = false;
    CONSOLE_CURSOR_INFO oldCursor{};
    DWORD originalOutputMode = 0;
    bool restoreOutputMode = false;
    bool vtEnabled = false;
    wstring sequence;
};
