#include "Cell.h"

Cell::Cell(int pieceId, int durability)
    : pieceId(pieceId), durability(durability)
{
}

int Cell::getPieceId() const
{
    return pieceId;
}

int Cell::getDurability() const
{
    return durability;
}

void Cell::weaken()
{
    if (durability > 0) --durability;
}
