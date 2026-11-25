#include "PlacedBlock.h"

PlacedBlock::PlacedBlock(BlockSpec spec, int remaining, bool intact)
    : spec(spec), remaining(remaining), intact(intact)
{
}

const BlockSpec& PlacedBlock::getSpec() const
{
    return spec;
}

bool PlacedBlock::getIntact() const
{
    return intact;
}

bool PlacedBlock::loseCell()
{
    intact = false;
    return --remaining == 0;
}

void PlacedBlock::removeReinforcement()
{
    spec.setReinforcedCell(-1);
}
