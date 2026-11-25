#include "GameManager.h"
#include <random>
#include <thread>

using namespace std;

double GameManager::elapsedSince(chrono::steady_clock::time_point& last)
{
    const auto now = chrono::steady_clock::now();
    const auto duration = chrono::duration<double, milli>(now - last);
    last = now;
    return duration.count();
}

void GameManager::playSingle(StageManager::Mode mode, bool debug)
{
    StageManager game;
    game.start(mode, debug, random_device{}());
    bool help = false;
    auto last = chrono::steady_clock::now();
    display.showGame(game, help);

    for (;;) {
        const double elapsedMs = elapsedSince(last);
        bool redraw = false;
        bool skip_tick = help;
        if (input.keyAvailable()) {
            const int key = input.readKey();
            const bool extended = key == 0 || key == 224;
            const int code = extended ? input.readKey() : key;
            if (!extended && code == 27) break;
            if (game.finished()) {
                if (!extended && (code == 'r' || code == 'R')) {
                    game.start(mode, debug, random_device{}());
                    help = false;
                    skip_tick = true;
                    input.discardPendingKeys();
                }
            } else if (!extended && (code == 'h' || code == 'H')) {
                help = !help;
                skip_tick = true;
                input.discardPendingKeys();
            } else if (!help && game.getPhase() == StageManager::Phase::Choosing) {
                if (extended && code == 0x48) game.selectCard(-1);
                if (extended && code == 0x50) game.selectCard(1);
                if (!extended && code == ' ') {
                    game.takeCard();
                    input.discardPendingKeys();
                }
                skip_tick = true;
            } else if (!help && extended) {
                switch (code) {
                case 0x48: game.tryMove(0, true); break;
                case 0x4b: game.tryMove(-1, false); break;
                case 0x4d: game.tryMove(1, false); break;
                case 0x50: game.fallStep(); break;
                }
            } else if (!help) {
                if (code == ' ') game.hardDrop();
                else if (code == 'c' || code == 'C') skip_tick = game.tryHold();
            }
            redraw = true;
        }
        if (!help && !skip_tick) redraw = game.tick(elapsedMs) || redraw;
        if (game.getPhase() == StageManager::Phase::TurnEnd) {
            game.finishTurn(game.difficulty());
            redraw = true;
        }
        if (redraw) display.showGame(game, help);
        this_thread::sleep_for(chrono::milliseconds(15));
    }
}

void GameManager::applyLocalKey(StageManager& first, StageManager& second, InputManager::LocalKey key)
{
    using LocalKey = InputManager::LocalKey;
    if (first.getPhase() == StageManager::Phase::Choosing) {
        if (key == LocalKey::FirstRotate) { first.selectCard(-1); return; }
        if (key == LocalKey::FirstSoftDrop) { first.selectCard(1); return; }
        if (key == LocalKey::FirstHardDrop) { first.takeCard(); return; }
    }
    if (second.getPhase() == StageManager::Phase::Choosing) {
        if (key == LocalKey::SecondRotate) { second.selectCard(-1); return; }
        if (key == LocalKey::SecondSoftDrop) { second.selectCard(1); return; }
        if (key == LocalKey::SecondHardDrop) { second.takeCard(); return; }
    }
    switch (key) {
    case LocalKey::FirstRotate: first.tryMove(0, true); break;
    case LocalKey::FirstLeft: first.tryMove(-1, false); break;
    case LocalKey::FirstSoftDrop: first.fallStep(); break;
    case LocalKey::FirstRight: first.tryMove(1, false); break;
    case LocalKey::FirstHold: first.tryHold(); break;
    case LocalKey::FirstHardDrop: first.hardDrop(); break;
    case LocalKey::SecondRotate: second.tryMove(0, true); break;
    case LocalKey::SecondLeft: second.tryMove(-1, false); break;
    case LocalKey::SecondSoftDrop: second.fallStep(); break;
    case LocalKey::SecondRight: second.tryMove(1, false); break;
    case LocalKey::SecondHold: second.tryHold(); break;
    case LocalKey::SecondHardDrop: second.hardDrop(); break;
    default: break;
    }
}

void GameManager::runTwoPlayer()
{
    StageManager first, second;
    first.start(StageManager::Mode::Local, false, random_device{}());
    second.start(StageManager::Mode::Local, false, random_device{}());
    display.showTwoPlayerGame(first, second);
    auto last = chrono::steady_clock::now();
    for (;;) {
        double elapsedMs = elapsedSince(last);
        bool redraw = false;
        for (int i = 0; i < 32; ++i) {
            const auto key = input.readLocalKey();
            if (key == InputManager::LocalKey::None) break;
            if (key == InputManager::LocalKey::Ignored) continue;
            if (key == InputManager::LocalKey::Escape) return;
            if (key == InputManager::LocalKey::Restart && first.finished() && second.finished()) {
                first.start(StageManager::Mode::Local, false, random_device{}());
                second.start(StageManager::Mode::Local, false, random_device{}());
                elapsedMs = 0;
            } else if (!first.finished() && !second.finished()) {
                applyLocalKey(first, second, key);
            }
            redraw = true;
        }
        if (!first.finished() && !second.finished()) {
            redraw = first.tick(elapsedMs) || redraw;
            redraw = second.tick(elapsedMs) || redraw;
            const int level = first.sharedDifficulty(second);
            first.finishTurn(level);
            second.finishTurn(level);
        }
        if (first.finished() || second.finished()) {
            redraw = redraw || first.getPhase() != StageManager::Phase::GameOver || second.getPhase() != StageManager::Phase::GameOver;
            first.endGame();
            second.endGame();
        }
        if (redraw) display.showTwoPlayerGame(first, second);
        this_thread::sleep_for(chrono::milliseconds(15));
    }
}

void GameManager::runStory(bool debug)
{
    playSingle(StageManager::Mode::Story, debug);
}

void GameManager::run()
{
    for (;;) {
        display.showMainMenu();
        for (;;) {
            if (!input.keyAvailable()) {
                this_thread::sleep_for(chrono::milliseconds(15));
                continue;
            }
            const int key = input.readKey();
            if (key == 0 || key == 224) {
                input.readKey();
                continue;
            }
            if (key == 27) return;
            if (key < '1' || key > '3') continue;
            input.discardPendingKeys();
            if (key == '2') runTwoPlayer();
            else playSingle(key == '1' ? StageManager::Mode::Story : StageManager::Mode::Endless, false);
            input.discardPendingKeys();
            break;
        }
    }
}
