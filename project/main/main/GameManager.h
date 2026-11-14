#pragma once

#include "BlockManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "StageManager.h"

class GameManager
{
public:
    void run();

private:
    BlockManager bm;
    DisplayManager dm;
    InputManager im;
    StageManager sm;
    Block current;
    int next_shape = 0;

    bool try_move(const Block& candidate);
    int move_block();
};
