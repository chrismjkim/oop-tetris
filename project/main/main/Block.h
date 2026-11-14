#pragma once

class Block
{
public:
    int shape = 0;
    int angle = 0;
    int x = 5;
    int y = -3;

    static constexpr int size = 4;
    static constexpr int shape_count = 7;

    bool occupied(int row, int column) const;
};
