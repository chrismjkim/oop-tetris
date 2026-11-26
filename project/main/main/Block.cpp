#include "Block.h"

Block::Block() : blockid(0) {}

Block::Block(BlockType type, int blockid) : type(type), angle(Angles::NORTH), row(0), col(5), blockid(blockid) {
	auto it = BLOCKMAP.find(type);

	int color = 1;
		// TODO: type 받아서 컬러 반환하는 함수 추가 필요
	if (it != BLOCKMAP.end()) {
		auto arr = it->second;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++)
			{
				cells[i][j] = arr[i][j] ? new Cell(blockid, color) : nullptr; 
			}
		}
	}

}



array<array<Cell*, 4>, 4>& Block::getCells() {
	return cells;
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