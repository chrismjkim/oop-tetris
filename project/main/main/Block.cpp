#include "Block.h"

Block::Block(BlockType type) : type(type), angle(Angles::NORTH), posX(3), posY(0) {
	auto it = BLOCKMAP.find(type);
	if (it != BLOCKMAP.end()) {
		fill = it->second;
	}

}

array<array<int, 4>, 4>& Block::getFill() {
	return fill;
}

const array<array<int, 4>, 4>& Block::getFill() const {
	return fill;
}

void Block::setPos(const int x, const int y) {
	posX = x;
	posY = y;
}

int Block::getX() const {
	return posX;
}

int Block::getY() const {
	return posY;
}

Angles Block::getAngle() const {
	return angle;
}



BlockType Block::getType() const {
	return type;
}

// 행렬 90도 회전
array<array<int, 4>, 4> Block::rotate90(const array<array<int, 4>, 4>& src) {
	array<array<int, 4>, 4> dst{};

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			dst[c][3 - r] = src[r][c];
		}
	}
	return dst;

}


bool Block::rotateBlock(Angles newAngle) {
	int angleDiff = static_cast<int>(newAngle) - static_cast<int>(angle);
	if (angleDiff < 0) angleDiff += 360;
	if (angleDiff % 90 != 0) return false;

	array<array<int, 4>, 4> dst = fill;

	for (int i = 0; i < angleDiff / 90; i++) {
		dst = rotate90(dst);
	}

	fill = dst;
	angle = newAngle;
	return true;
}
