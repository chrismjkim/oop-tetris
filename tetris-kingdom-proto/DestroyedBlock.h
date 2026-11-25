#pragma once

#include "BlockSpec.h"

class DestroyedBlock {
public:
    DestroyedBlock(int id, BlockSpec spec);
    int getId() const;
    const BlockSpec& getSpec() const;

private:
    int id;
    BlockSpec spec;
};
