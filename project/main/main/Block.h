#pragma once

#include <array>
#include <unordered_map>
#include "BlockType.h"
#include "Angles.h"


using std::array;
using std::unordered_map;

class Block
{
private:
	BlockType type;
	Angles angle;
	int posX, posY;
	array<array<int, 4>, 4> fill;
	array<array<int, 4>, 4> rotate90(const array<array<int,4>, 4>& src);


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
		{1, 1, 1, 0},
		{0, 1, 0, 0},
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
	Block() = default;
	Block(BlockType type);
	BlockType getType() const;
	void setPos(const int x, const int y);
	int getX();
	int getY();

	array<array<int, 4>, 4>& getFill();
	bool rotateBlock(Angles angle);


};

