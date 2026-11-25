#include "RemovedCell.h"

RemovedCell::RemovedCell(Point position, Cell cell)
    : position(position), cell(cell)
{
}

const Point& RemovedCell::getPosition() const
{
    return position;
}

const Cell& RemovedCell::getCell() const
{
    return cell;
}
