#include "StageRules.h"

using namespace std;

StageRules::StageRules(int cellsPerSecond, int thiefPercent, int reinforcePercent, int giantBagPercent, bool thiefCanTakeGiant)
    : cellsPerSecond(cellsPerSecond), thiefPercent(thiefPercent), reinforcePercent(reinforcePercent), giantBagPercent(giantBagPercent), thiefCanTakeGiant(thiefCanTakeGiant)
{
}

int StageRules::getCellsPerSecond() const
{
    return cellsPerSecond;
}

int StageRules::getThiefPercent() const
{
    return thiefPercent;
}

int StageRules::getReinforcePercent() const
{
    return reinforcePercent;
}

int StageRules::getGiantBagPercent() const
{
    return giantBagPercent;
}

bool StageRules::getThiefCanTakeGiant() const
{
    return thiefCanTakeGiant;
}

void StageRules::requireNormalSize(int shape)
{
    normalSizeOnly[shape] = true;
}

bool StageRules::requiresNormalSize(int shape) const
{
    return normalSizeOnly[shape];
}
