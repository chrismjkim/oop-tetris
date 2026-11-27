#pragma once
#include "CellType.h"

class Cell
{
private: 
	CellState type;
	const int blockid;
	int color;
public:
	Cell()=default;
	Cell(int id, int color);
	Cell(CellState type, int id, int color);
	void setType(CellState type);
	CellState getType();
	int getBlockId() const;
	int getColor();

};

