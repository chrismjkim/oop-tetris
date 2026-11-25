#include "Card.h"

Card::Card(Kind kind, int shape) : kind(kind), shape(shape)
{
}

Card::Kind Card::getKind() const
{
    return kind;
}

int Card::getShape() const
{
    return shape;
}

bool Card::repeatable() const
{
    return kind == Kind::Filler || kind == Kind::Bonus || kind == Kind::Steady;
}
