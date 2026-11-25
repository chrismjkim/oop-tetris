#include "game.h"
#include "console.h"
#include "mechanics.h"
#include "render.h"

#include <chrono>
#include <random>

namespace tetris {
namespace {

using Clock = std::chrono::steady_clock;

double elapsed_since(Clock::time_point& last)
{
    const auto now = Clock::now();
    const auto duration = std::chrono::duration<double, std::milli>(now - last);
    last = now;
    return duration.count();
}

bool finished(const GameState& game)
{
    return game.phase == Phase::GameOver || game.phase == Phase::Victory;
}

void play_single(Mode mode, bool debug)
{
    GameState game;
    start_run(game, mode, debug, std::random_device{}());
    bool help = false;
    auto last = Clock::now();
    render::show_game(game, help);

    for (;;) {
        const double elapsed_ms = elapsed_since(last);
        bool redraw = false;
        bool skip_tick = help;
        if (console::key_available()) {
            const int key = console::read_key();
            const bool extended = key == 0 || key == 224;
            const int code = extended ? console::read_key() : key;
            if (!extended && code == 27) break;
            if (finished(game)) {
                if (!extended && (code == 'r' || code == 'R')) {
                    start_run(game, mode, debug, std::random_device{}());
                    help = false;
                    skip_tick = true;
                    console::discard_pending_keys();
                }
            } else if (!extended && (code == 'h' || code == 'H')) {
                help = !help;
                skip_tick = true;
                console::discard_pending_keys();
            } else if (!help && game.phase == Phase::Choosing) {
                if (extended && code == 0x48) select_card(game, -1);
                if (extended && code == 0x50) select_card(game, 1);
                if (!extended && code == ' ') {
                    take_card(game);
                    console::discard_pending_keys();
                }
                skip_tick = true;
            } else if (!help && extended) {
                switch (code) {
                case 0x48: try_move(game, 0, true); break;
                case 0x4b: try_move(game, -1, false); break;
                case 0x4d: try_move(game, 1, false); break;
                case 0x50: fall_step(game); break;
                }
            } else if (!help) {
                if (code == ' ') hard_drop(game);
                else if (code == 'c' || code == 'C') skip_tick = try_hold(game);
            }
            redraw = true;
        }
        if (!help && !skip_tick) redraw = advance_tick(game, elapsed_ms) || redraw;
        if (game.phase == Phase::TurnEnd) {
            finish_turn(game, difficulty_for(game));
            redraw = true;
        }
        if (redraw) render::show_game(game, help);
        console::sleep_ms(15);
    }
}

void apply_local_key(GameState& first, GameState& second, console::LocalKey key)
{
    using console::LocalKey;
    if (first.phase == Phase::Choosing) {
        if (key == LocalKey::FirstRotate) { select_card(first, -1); return; }
        if (key == LocalKey::FirstSoftDrop) { select_card(first, 1); return; }
        if (key == LocalKey::FirstHardDrop) { take_card(first); return; }
    }
    if (second.phase == Phase::Choosing) {
        if (key == LocalKey::SecondRotate) { select_card(second, -1); return; }
        if (key == LocalKey::SecondSoftDrop) { select_card(second, 1); return; }
        if (key == LocalKey::SecondHardDrop) { take_card(second); return; }
    }
    switch (key) {
    case LocalKey::FirstRotate: try_move(first, 0, true); break;
    case LocalKey::FirstLeft: try_move(first, -1, false); break;
    case LocalKey::FirstSoftDrop: fall_step(first); break;
    case LocalKey::FirstRight: try_move(first, 1, false); break;
    case LocalKey::FirstHold: try_hold(first); break;
    case LocalKey::FirstHardDrop: hard_drop(first); break;
    case LocalKey::SecondRotate: try_move(second, 0, true); break;
    case LocalKey::SecondLeft: try_move(second, -1, false); break;
    case LocalKey::SecondSoftDrop: fall_step(second); break;
    case LocalKey::SecondRight: try_move(second, 1, false); break;
    case LocalKey::SecondHold: try_hold(second); break;
    case LocalKey::SecondHardDrop: hard_drop(second); break;
    default: break;
    }
}

void play_two_player()
{
    GameState first, second;
    start_run(first, Mode::Local, false, std::random_device{}());
    start_run(second, Mode::Local, false, std::random_device{}());
    render::show_two_player_game(first, second);
    auto last = Clock::now();
    for (;;) {
        double elapsed_ms = elapsed_since(last);
        bool redraw = false;
        for (int i = 0; i < 32; ++i) {
            const auto key = console::read_local_key();
            if (key == console::LocalKey::None) break;
            if (key == console::LocalKey::Ignored) continue;
            if (key == console::LocalKey::Escape) return;
            if (key == console::LocalKey::Restart && finished(first) && finished(second)) {
                start_run(first, Mode::Local, false, std::random_device{}());
                start_run(second, Mode::Local, false, std::random_device{}());
                elapsed_ms = 0;
            } else if (!finished(first) && !finished(second)) {
                apply_local_key(first, second, key);
            }
            redraw = true;
        }
        if (!finished(first) && !finished(second)) {
            redraw = advance_tick(first, elapsed_ms) || redraw;
            redraw = advance_tick(second, elapsed_ms) || redraw;
            const int level = shared_difficulty(first, second);
            finish_turn(first, level);
            finish_turn(second, level);
        }
        if (finished(first) || finished(second)) {
            redraw = redraw || first.phase != Phase::GameOver || second.phase != Phase::GameOver;
            first.phase = second.phase = Phase::GameOver;
        }
        if (redraw) render::show_two_player_game(first, second);
        console::sleep_ms(15);
    }
}

} // namespace

void run_game(bool debug)
{
    console::initialize();
    play_single(Mode::Story, debug);
    console::shutdown();
}

void run_two_player_game()
{
    console::initialize();
    play_two_player();
    console::shutdown();
}

void run_main_menu()
{
    console::initialize();
    for (;;) {
        render::show_main_menu();
        for (;;) {
            if (!console::key_available()) {
                console::sleep_ms(15);
                continue;
            }
            const int key = console::read_key();
            if (key == 0 || key == 224) {
                console::read_key();
                continue;
            }
            if (key == 27) {
                console::shutdown();
                return;
            }
            if (key < '1' || key > '3') continue;
            console::discard_pending_keys();
            if (key == '2') play_two_player();
            else play_single(key == '1' ? Mode::Story : Mode::Endless, false);
            console::discard_pending_keys();
            break;
        }
    }
}

} // namespace tetris
