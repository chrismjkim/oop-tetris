#include "LineClear.h"

using namespace std;

int LineClear::lineCount() const
{
    return lines;
}

const vector<RemovedCell>& LineClear::removedCells() const
{
    return removed;
}

const vector<DestroyedBlock>& LineClear::destroyedBlocks() const
{
    return destroyed;
}
