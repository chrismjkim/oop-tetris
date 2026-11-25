#pragma once

#include "Point.h"
#include "BlockSpec.h"
#include <array>

using namespace std;

class Block {
public:
    static constexpr int shapeCount = 7;
    Block(BlockSpec spec = {}, Point origin = {}, int angle = 0);
    static Block spawn(BlockSpec spec);
    static const char* shapeName(int shape);
    array<Point, 16> cells() const;
    const BlockSpec& specification() const;
    Point position() const;
    Block shifted(int dx, int dy) const;
    Block rotated() const;

private:
    BlockSpec spec;
    Point origin;
    int angle;
};
