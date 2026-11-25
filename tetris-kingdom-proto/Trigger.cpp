#include "Trigger.h"
#include <utility>

using namespace std;

Trigger::Trigger(Card::Kind kind, int repeats, vector<int> neighbors)
    : kind(kind), repeats(repeats), neighbors(move(neighbors))
{
}

Card::Kind Trigger::getKind() const
{
    return kind;
}

const vector<int>& Trigger::getNeighbors() const
{
    return neighbors;
}

bool Trigger::repeat()
{
    return --repeats > 0;
}
