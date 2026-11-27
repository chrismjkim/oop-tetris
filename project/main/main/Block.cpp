#include "Block.h"
#include "Colors.h"

Block::Block() = default;

Block::Block(BlockType type, int blockid) : type(type), blockid(blockid) {
	auto it = BLOCKMAP.find(type);

	if (it != BLOCKMAP.end()) {

		static constexpr Colors colors[] = {
			Colors::BLOCK_I, Colors::BLOCK_O, Colors::BLOCK_T, Colors::BLOCK_J,
			Colors::BLOCK_L, Colors::BLOCK_S, Colors::BLOCK_Z
		};
		const int color = static_cast<int>(colors[static_cast<int>(type)]);

		auto arr = it->second;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++)
			{
				cells[i][j] = arr[i][j] ? new Cell(blockid, color) : nullptr; 
			}
		}
	}
	spawnCells = cells;
}



array<array<Cell*, 4>, 4>& Block::getCells() {
	return cells;
}

const array<array<Cell*, 4>, 4>& Block::getCells() const {
	return cells;
}

void Block::resetForSpawn() {
	row = spawnRow;
	col = spawnCol;
	cells = spawnCells;
}

void Block::setPos(const int row, const int col) {
	if (row > 20 || col > 10) {
		throw;
	}
	this->row = row;
	this->col = col;
}

int Block::getRow() {
	return row;
}

int Block::getCol() {
	return col;
}

int Block::getBlockid() const {
	return blockid;
}


BlockType Block::getType() const {
	return type;
}


array<array<Cell*, 4>, 4> Block::rotate90(const array<array<Cell*, 4>, 4>& src) const {
	if (type == BlockType::T) {
		array<array<Cell*, 4>, 4> res{};
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				res[j][2 - i] = src[i][j];
			}
		}
		return res;
	}

	array<array<Cell*, 4>, 4> temp{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			temp[j][3 - i] = src[i][j];
		}
	}

	int minR = 4, minC = 4;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (temp[i][j] != nullptr) {
				if (i < minR) minR = i;
				if (j < minC) minC = j;
			}
		}
	}

	array<array<Cell*, 4>, 4> res{};
	for (int i = minR; i < 4; i++) {
		for (int j = minC; j < 4; j++) {
			res[i - minR][j - minC] = temp[i][j];
		}
	}

	return res;
}
