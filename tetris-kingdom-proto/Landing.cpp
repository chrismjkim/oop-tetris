#include "Landing.h"

void Landing::update(int y, bool supported)
{
    if (y > lowestY) {
        lowestY = y;
        movesLeft = Landing::moveLimit;
    }
    grounded = supported;
    if (!grounded) return;
    if (!touched || y > lastContactY) timeLeftMs = Landing::delayMs;
    touched = true;
    lastContactY = y;
}

void Landing::reset(int y)
{
    *this = {};
    lowestY = y;
}

void Landing::useMove()
{
    if (touched && movesLeft > 0) --movesLeft;
}

void Landing::resetTimer()
{
    timeLeftMs = delayMs;
}

void Landing::wait(double elapsed)
{
    timeLeftMs -= elapsed;
}

bool Landing::expired() const
{
    return timeLeftMs <= 0 || outOfMoves();
}

bool Landing::groundedNow() const
{
    return grounded;
}

bool Landing::outOfMoves() const
{
    return movesLeft == 0;
}
