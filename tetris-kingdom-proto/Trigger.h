#pragma once

#include "Card.h"
#include <vector>

using namespace std;

class Trigger {
public:
    Trigger(Card::Kind kind = Card::Kind::Filler, int repeats = 1, vector<int> neighbors = {});
    Card::Kind getKind() const;
    const vector<int>& getNeighbors() const;
    bool repeat();

private:
    Card::Kind kind;
    int repeats;
    vector<int> neighbors;
};
