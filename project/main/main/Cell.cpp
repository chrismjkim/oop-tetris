#include "Cell.h"
Cell::Cell(int id, int color) : Cell(CellState::NORMAL, id, color) {}

Cell::Cell(CellState type, int id, int color) : type(type), blockid(id), color(color) {}


CellState Cell::getType() {
	return type;
}

void Cell::setType(CellState type) {
	this->type = type;
}

int Cell::getBlockId() const {
	return blockid;
}

int Cell::getColor() {
	return color;
}

