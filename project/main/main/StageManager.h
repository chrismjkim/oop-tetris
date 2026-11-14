#pragma once

class StageManager
{
public:
    int level = 0;
    int score = 0;
    int lines = 0;

    int speed() const;
    int stick_rate() const;
    int clear_line() const;
    void add_line();
    void advance_if_cleared();
};
