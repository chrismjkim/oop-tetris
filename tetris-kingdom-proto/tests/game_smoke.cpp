#include "../console.h"
#include "../game.h"
#include "../mechanics.h"
#include "../render.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace {

char screen[tetris::console::kFrameRows][tetris::console::kFrameColumns + 1];
int key_reads = 0;
int local_reads = 0;
int frames = 0;

} // namespace

namespace tetris {
namespace console {

void initialize() {}
void shutdown() {}
void begin_frame()
{
    for (auto& row : screen) {
        std::memset(row, ' ', kFrameColumns);
        row[kFrameColumns] = '\0';
    }
}
void end_frame() { ++frames; }
void write_text(int x, int y, int color, const char* value)
{
    const auto length = std::strlen(value);
    assert(x >= 0 && y >= 0 && y < kFrameRows && x + length <= kFrameColumns);
    assert(color >= BLACK && color <= BLOCK_S);
    std::memcpy(&screen[y][x], value, length);
}
void write_block(int x, int y, int color, BlockStyle style)
{
    switch (style) {
    case BlockStyle::Solid: write_text(x, y, color, "[]"); break;
    case BlockStyle::Reinforced: write_text(x, y, color, "@]"); break;
    case BlockStyle::CompactReinforced: write_text(x, y, color, "@"); break;
    }
}
void write_wide_text(int x, int y, int color, const wchar_t* value)
{
    for (; *value; ++value) {
        const int width = glyph_columns(*value);
        write_text(x, y, color, width == 2 ? "##" : "*");
        x += width;
    }
}
void sleep_ms(unsigned) {}
bool key_available() { return true; }
int read_key()
{
    constexpr int script[] = {224, 0x50, ' ', ' ', 'h', 'h', 27};
    assert(key_reads < static_cast<int>(sizeof(script) / sizeof(script[0])));
    return script[key_reads++];
}
LocalKey read_local_key()
{
    constexpr LocalKey script[] = {LocalKey::FirstHardDrop, LocalKey::SecondHardDrop,
        LocalKey::None, LocalKey::Escape};
    assert(local_reads < static_cast<int>(sizeof(script) / sizeof(script[0])));
    return script[local_reads++];
}
void discard_pending_keys() {}

} // namespace console
} // namespace tetris

int main()
{
    using namespace tetris;
    GameState game, second;
    for (int i = 0; i < static_cast<int>(Augment::Count); ++i) {
        const auto& info = card_info(static_cast<Augment>(i));
        for (auto line : {info.name, info.line1, info.line2}) {
            int columns = info.shape_specific && line == info.name ? 2 : 0;
            for (const wchar_t* ch = line; *ch; ++ch) columns += console::glyph_columns(*ch);
            assert(columns <= 22);
        }
    }
    start_run(game, Mode::Story, false, 3);
    render::show_game(game);
    assert(std::strstr(screen[4], "CHOOSE AUGMENT") && std::strstr(screen[8], "===="));
    take_card(game);
    game.next = {0, true, 15};
    game.has_hold = true;
    game.hold = {1, false, 0};
    game.current = {{0, false, 3}, {4, -3}, 0};
    game.placed.emplace(99, PlacedPiece{{0, false, 0}, 4, true});
    game.board[10][0] = {99, 2};
    game.board[11][0] = {99, 1};
    game.warning_id = 99;
    game.theft_outcome = TheftOutcome::Warned;
    render::show_game(game);
    assert(std::strstr(screen[0], "STAGE 1/4"));
    assert(std::strncmp(&screen[17][15], "##", 2) == 0);
    assert(std::strncmp(&screen[15][39], "#@ ", 3) == 0); // 거대 NEXT의 한 문자 칸
    assert(std::strncmp(&screen[9][4], "@]", 2) == 0);   // HOLD
    assert(std::strncmp(&screen[7][23], "@]", 2) == 0);  // 보드에 진입한 강화 칸
    assert(std::strncmp(&screen[26][23], "[]", 2) == 0); // 착지 미리보기
    game.warning_id = 0;
    render::show_game(game);
    assert(std::strncmp(&screen[17][15], "@]", 2) == 0);
    game.board[10][0].durability = 1;
    render::show_game(game);
    assert(std::strncmp(&screen[17][15], "[]", 2) == 0);

    start_run(second, Mode::Local, false, 7);
    render::show_two_player_game(game, second);
    assert(std::strstr(screen[0], "1P") && std::strstr(screen[0], "2P"));
    assert(std::strstr(screen[2], "LOCAL 2P"));
    second.total_lines = 20;
    second.phase = Phase::TurnEnd;
    finish_turn(second, shared_difficulty(game, second));
    render::show_two_player_game(game, second);
    assert(std::strstr(screen[4], "CHOOSE AUGMENT") == &screen[4][55]);
    assert(std::strstr(screen[28], "2P numpad 5/2: select"));
    render::show_main_menu();
    assert(std::strstr(screen[11], "1P Story"));
    assert(std::strstr(screen[13], "Local 2P"));
    assert(std::strstr(screen[15], "1P Endless"));

    run_game(true);
    run_two_player_game();
    assert(key_reads == 7 && local_reads == 4 && frames >= 8);
    std::puts("PASS: 1P and 2P game loops, giant NEXT, reinforcement and theft warning output");
}
