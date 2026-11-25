#include "BlockSpec.h"

BlockSpec::BlockSpec(int shape, bool giant, int reinforced)
    : shape(shape), giant(giant), reinforced(reinforced)
{
}

int BlockSpec::getShape() const
{
    return shape;
}

bool BlockSpec::getGiant() const
{
    return giant;
}

int BlockSpec::getReinforced() const
{
    return reinforced;
}

int BlockSpec::size() const
{
    return giant ? 16 : 4;
}

void BlockSpec::setGiant(bool value)
{
    giant = value;
}

void BlockSpec::setReinforcedCell(int index)
{
    reinforced = index;
}
