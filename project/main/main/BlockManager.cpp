#include "BlockManager.h"
#include <stdlib.h>

BlockManager::BlockManager() : blockList(), cellList{} {

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
	const auto& arr = block.getFill();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			const int x = block.getX() + i;
			const int y = block.getY() + j;
			if (arr[i][j] == 1 && x >= 0 && x < COL && y >= 0 && y < ROW) {
				cellList[y][x] = static_cast<int>(block.getType()) + 1;
			}
		}
	}
}

int BlockManager::clearFullRows() {
	int clearedRows = 0;
	for (int row = ROW - 1; row >= 0; --row) {
		bool isFull = true;
		for (int col = 0; col < COL; ++col) {
			if (cellList[row][col] == 0) {
				isFull = false;
				break;
			}
		}

		if (!isFull) {
			continue;
		}

		++clearedRows;
		for (int moveRow = row; moveRow > 0; --moveRow) {
			cellList[moveRow] = cellList[moveRow - 1];
		}
		cellList[0] = {};
		++row;
	}
	return clearedRows;
}

bool BlockManager::canPlace(const Block& block) const {
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
