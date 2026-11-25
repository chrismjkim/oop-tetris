#include "DestroyedBlock.h"

DestroyedBlock::DestroyedBlock(int id, BlockSpec spec)
    : id(id), spec(spec)
{
}

int DestroyedBlock::getId() const
{
    return id;
}

const BlockSpec& DestroyedBlock::getSpec() const
{
    return spec;
}
