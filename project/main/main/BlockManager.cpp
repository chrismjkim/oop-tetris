#include "BlockManager.h"
#include <stdlib.h>

#include <iostream>
#include <Windows.h>
#include <string>

using std::cout;
using std::endl;

BlockManager::BlockManager() : blockList(50), cellList{} {

}

Block BlockManager::make_new_block(int stick_rate) {
	int shape;
	int i;
	i = rand() % 100;
	if (i <= stick_rate)		//막대기 나올확률 계산
		return BlockType::I;							//막대기 모양으로 리턴

	shape = (rand() % 6) + 1;		//shape에는 1~6의 값이 들어감
	BlockType type = static_cast<BlockType>(shape);
	Block bl(type);
	return bl;
}

void BlockManager::appendBlock(Block& block) {
	blockList.push_back(block);
	const auto arr = block.getFill();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cellList[defBlockX + i][defBlockY] = arr[i][j];
		}
	}
}


bool BlockManager::canPlace(Block& block) const {
	const auto& cells = block.getFill();
	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			if (cells[x][y] == 0) {
				continue;
			}

			const int boardX = block.getX() + x;
			const int boardY = block.getY() + y;
			if (boardX < 0 || boardX >= COL || boardY < 0 || boardY >= ROW) {
				return false;
			}
			if (cellList[boardY][boardX] != 0) {
				return false;
			}
		}
	}
	return true;
}

void BlockManager::resetBlocks() {
	cellList = {};
	blockList = {};
}

const array<array<int, BlockManager::COL>, BlockManager::ROW>& BlockManager::getCellList() const {
	return cellList;
}

const vector<Block>& BlockManager::getBlockList() const {
	return blockList;
}

bool BlockManager::downBlock(Block& currentBlock) {
	int x = currentBlock.getX();
	int y = currentBlock.getY();

	if (y == 14) {
		return false;
	}

	for (int i = 0; i < 4; i++) {
		
		if (cellList[x + i][max(y + 4, 9)] && currentBlock.getFill()[i][3]) {
			return false;
		}
	}
	currentBlock.setPos(x, ++y);
	return true;
}