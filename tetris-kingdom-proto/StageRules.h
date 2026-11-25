#pragma once

#include "Block.h"
#include <array>

using namespace std;

class StageRules {
public:
    StageRules(int cellsPerSecond, int thiefPercent, int reinforcePercent,
               int giantBagPercent, bool thiefCanTakeGiant = false);
    int getCellsPerSecond() const;
    int getThiefPercent() const;
    int getReinforcePercent() const;
    int getGiantBagPercent() const;
    bool getThiefCanTakeGiant() const;
    void requireNormalSize(int shape);
    bool requiresNormalSize(int shape) const;

private:
    array<bool, Block::shapeCount> normalSizeOnly{};
    int cellsPerSecond;
    int thiefPercent;
    int reinforcePercent;
    int giantBagPercent;
    bool thiefCanTakeGiant;
};
