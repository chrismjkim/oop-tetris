#pragma once

#include "Point.h"
#include "Cell.h"

class RemovedCell {
public:
    RemovedCell(Point position, Cell cell);
    const Point& getPosition() const;
    const Cell& getCell() const;

private:
    Point position;
    Cell cell;
};
