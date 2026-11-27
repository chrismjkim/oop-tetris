#pragma once

#include <array>
#include <unordered_map>
#include "Cell.h"
#include "BlockType.h"


using std::array;
using std::unordered_map;

class Block
{
private:
	BlockType type;
	int blockid;
	int row, col; // Cells 20 by 10 기준 상대위치
	array<array<Cell*, 4>, 4> cells = {};


	static inline const std::unordered_map<BlockType, array<array<int, 4>, 4>> BLOCKMAP = {
	{BlockType::I, {{
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
	}}},
	{BlockType::O, {{
		{1, 1, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	}}},
	{BlockType::T, {{
		{0, 1, 0, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	}}},
	{BlockType::J, {{
		{0, 1, 0, 0},
		{0, 1, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},
	} }},
	{BlockType::L, {{
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},
	} }},
	{BlockType::S, {{
		{0, 1, 1, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	}}},
	{BlockType::Z, {{
		{1, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
	}}},
	};

public:
	Block();
	Block(BlockType type, int blockid);
	BlockType getType() const;
	void setPos(const int row, const int col);
	int getRow();
	int getCol();
	int getBlockid() const;
	

	array<array<Cell*, 4>, 4>& getCells();
	array<array<Cell*, 4>, 4> rotate90(const array<array<Cell*, 4>, 4>& src) const;


};

