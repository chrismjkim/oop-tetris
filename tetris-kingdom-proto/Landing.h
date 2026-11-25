#pragma once

class Landing {
public:
    static constexpr int delayMs = 500;
    static constexpr int moveLimit = 15;
    void reset(int y);
    void update(int y, bool supported);
    void useMove();
    void resetTimer();
    void wait(double elapsed);
    bool expired() const;
    bool groundedNow() const;
    bool outOfMoves() const;

private:
    int lowestY = 0;
    int lastContactY = 0;
    int movesLeft = moveLimit;
    double timeLeftMs = delayMs;
    bool touched = false;
    bool grounded = false;
};
