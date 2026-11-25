#pragma once

class Point {
public:
    Point(int x = 0, int y = 0);
    int getX() const;
    int getY() const;

private:
    int x;
    int y;
};
