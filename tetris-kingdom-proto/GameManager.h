#pragma once

#include "DisplayManager.h"
#include "InputManager.h"
#include <chrono>

using namespace std;

class GameManager {
public:
    void run();
    void runStory(bool debug);
    void runTwoPlayer();

private:
    DisplayManager display;
    InputManager input;
    static double elapsedSince(chrono::steady_clock::time_point& last);
    static void applyLocalKey(StageManager& first, StageManager& second, InputManager::LocalKey key);
    void playSingle(StageManager::Mode mode, bool debug);
};
