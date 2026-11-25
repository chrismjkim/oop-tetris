#pragma once

#include "Block.h"
#include "StageRules.h"
#include <random>
#include <array>

using namespace std;

class BlockManager {
public:
    BlockSpec draw(mt19937& random, const StageRules& rules);

private:
    array<BlockSpec, Block::shapeCount> pieces{};
    int next = Block::shapeCount;
};
