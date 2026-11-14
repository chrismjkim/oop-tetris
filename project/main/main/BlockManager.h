#pragma once

#include "Block.h"

// 고정된 블록과 게임판의 벽, 바닥을 관리한다.
class BlockManager
{
public:
    static constexpr int rows = 21;
    static constexpr int columns = 14;

    void reset();
    int make_new_block(int stick_rate) const;
    bool occupied(int row, int column) const;
    bool strike_check(const Block& piece) const;
    bool merge_block(const Block& piece);
    bool check_full_line(int row) const;
    void clear_line(int row);

private:
    char total_block[rows][columns] = {};
};
