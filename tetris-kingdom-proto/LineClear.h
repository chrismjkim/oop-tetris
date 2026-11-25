#pragma once

#include "RemovedCell.h"
#include "DestroyedBlock.h"
#include <vector>

using namespace std;

class LineClear {
public:
    int lineCount() const;
    const vector<RemovedCell>& removedCells() const;
    const vector<DestroyedBlock>& destroyedBlocks() const;

private:
    int lines = 0;
    vector<RemovedCell> removed;
    vector<DestroyedBlock> destroyed;
    friend class Board;
};
