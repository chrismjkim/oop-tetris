#pragma once

#include "BlockSpec.h"

class PlacedBlock {
public:
    PlacedBlock(BlockSpec spec = {}, int remaining = 0, bool intact = true);
    const BlockSpec& getSpec() const;
    bool getIntact() const;
    bool loseCell();
    void removeReinforcement();

private:
    BlockSpec spec;
    int remaining;
    bool intact;
};
