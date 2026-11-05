#include "game.h"
#include "blocks.h"
#include "board.h"
#include "console.h"
#include "render.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace tetris {
namespace {

enum class FallResult { Moved, Locked, GameOver };

void reset_game(GameState& game)
{
    reset_board(game.board);
    game.level = 0;
    game.lines = 0;
    // 재시작할 때 점수를 유지하는 원본 동작을 보존한다.
}

void show_intro()
{
    render::show_logo();
    unsigned frame = 0;
    while (!console::key_available()) {
        if (frame % 40 == 0) {
            render::animate_logo();
        }
        ++frame;
        console::sleep_ms(30);
    }
    console::read_key();
    console::discard_pending_keys();
    console::clear();
}

bool select_start_level(GameState& game)
{
    render::show_controls();
    char input[128];
    while (true) {
        render::show_level_prompt();
        if (!std::fgets(input, sizeof(input), stdin)) {
            return false;
        }
        char* end = nullptr;
        const long choice = std::strtol(input, &end, 10);
        while (std::isspace(static_cast<unsigned char>(*end))) {
            ++end;
        }
        if (*end == '\0' && choice >= 1 && choice <= 8) {
            game.level = static_cast<int>(choice) - 1;
            console::clear();
            return true;
        }
    }
}

void check_full_lines(GameState& game)
{
    // 위에서부터 검사하면 줄 삭제로 내려오는 줄은 이미 검사한 상태다.
    for (int row = 0; row < kPlayRows; ++row) {
        if (!is_full_line(game.board, row)) {
            continue;
        }
        render::show_total_block(game);
        render::animate_full_line(row);
        remove_line(game.board, row);
        ++game.lines;
        game.score += 100 + game.level * 10 + std::rand() % 10;
        render::show_gamestat(game);
    }
}

FallResult move_block(GameState& game)
{
    render::erase_cur_block(game.current);
    // 한 칸 아래가 막혔으면 이동 전 위치에 블록을 고정한다.
    Piece candidate = game.current;
    ++candidate.y;
    if (!strike_check(game.board, candidate)) {
        game.current = candidate;
        return FallResult::Moved;
    }
    if (!merge_block(game.board, game.current)) {
        return FallResult::GameOver;
    }
    check_full_lines(game);
    render::show_total_block(game);
    // 미리보기 블록을 꺼내고 다음 미리보기를 채운다.
    game.current = make_start_piece(game.next_shape);
    game.next_shape = make_new_block(kStages[game.level].stick_rate);
    render::show_next_block(game);
    return FallResult::Locked;
}

void move_sideways_or_rotate(GameState& game, int dx, bool rotate)
{
    Piece candidate = game.current;
    candidate.x += dx;
    if (rotate) {
        candidate.angle = (candidate.angle + 1) % kRotationCount;
    }
    if (!strike_check(game.board, candidate)) {
        render::erase_cur_block(game.current);
        game.current = candidate;
        render::show_cur_block(game.current);
    }
}

bool handle_input(GameState& game)
{
    const int key = console::read_key();
    FallResult result = FallResult::Moved;
    if (key == 0 || key == 224) {
        switch (console::read_key()) {
        case 0x48: move_sideways_or_rotate(game, 0, true); break;
        case 0x4b: move_sideways_or_rotate(game, -1, false); break;
        case 0x4d: move_sideways_or_rotate(game, 1, false); break;
        case 0x50: result = move_block(game); break;
        }
    } else if (key == ' ') {
        // 이번 블록이 고정되거나 게임 오버가 되면 하드드롭을 끝낸다.
        do {
            result = move_block(game);
        } while (result == FallResult::Moved);
    }
    if (result != FallResult::GameOver) {
        render::show_cur_block(game.current);
    }
    return result == FallResult::GameOver;
}

void play_round(GameState& game)
{
    render::show_total_block(game);
    game.current = make_start_piece(make_new_block(kStages[game.level].stick_rate));
    game.next_shape = make_new_block(kStages[game.level].stick_rate);
    render::show_next_block(game);
    render::show_gamestat(game);

    for (unsigned tick = 1; ; ++tick) {
        bool game_over = false;
        if (console::key_available()) {
            game_over = handle_input(game);
        }
        // 입력에서 게임 오버가 났으면 같은 틱의 자동 낙하를 건너뛴다.
        if (!game_over && tick % kStages[game.level].speed == 0) {
            game_over = move_block(game) == FallResult::GameOver;
            if (!game_over) {
                render::show_cur_block(game.current);
            }
        }
        if (!game_over && game.level + 1 < kStageCount &&
            game.lines >= kStages[game.level].clear_line) {
            ++game.level;
            game.lines = 0;
            render::show_total_block(game);
            render::show_next_block(game);
            render::show_gamestat(game);
            render::show_cur_block(game.current);
        }
        if (game_over) {
            render::show_gameover();
            console::sleep_ms(1000);
            console::discard_pending_keys();
            console::read_key();
            console::discard_pending_keys();
            console::clear();
            console::set_color(console::GRAY);
            return;
        }
        console::gotoxy(77, 23);
        console::sleep_ms(15);
    }
}

} // namespace

void run_game()
{
    console::initialize();
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    GameState game;
    reset_game(game);
    show_intro();
    while (select_start_level(game)) {
        play_round(game);
        reset_game(game);
    }
    console::set_color(console::GRAY);
}

} // namespace tetris
