#include "BlockManager.h"
#include <stdlib.h>

#include "Util.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <utility>

using std::cout;
using std::endl;

static void releaseOffBoardCells(Block& block) {
	for (const auto& row : block.getCells()) {
		for (Cell* cell : row) {
			delete cell;
		}
	}
	block = Block{};
}

BlockManager::BlockManager() : cellList{} {

}

BlockManager::~BlockManager() {
	releaseOffBoardCells(heldBlock);
	releaseOffBoardCells(pendingSpawn);
}

bool BlockManager::isInside(const int row, const int col, const int i, const int j) const {
	// 좌표가 보드 범위 내에 있으면 true 리턴, OOB이면 false 리턴
	return row + i >= 0 && row + i < ROW && col + j >= 0 && col + j < COL;
}

void BlockManager::resetBlocks() {
	cellList = {};
	releaseOffBoardCells(heldBlock);
	releaseOffBoardCells(pendingSpawn);
	holdLockedBlockId = 0;
}

int BlockManager::clearFullLines() {
	int clearedLines = 0;

	for (int r = ROW - 1; r >= 0; r--) {
		bool isFull = true;

		for (int c = 0; c < COL; c++) {
			if (cellList[r][c] == nullptr) {
				isFull = false;
				break;
			}
		}

		if (isFull) {
			clearedLines++;

			for (int c = 0; c < COL; c++) {
				delete cellList[r][c];
				cellList[r][c] = nullptr;
			}

			for (int k = r; k > 0; k--) {
				cellList[k] = cellList[k - 1];
			}

			cellList[0].fill(nullptr);

			r++;
		}
	}

	return clearedLines; // 지워진 줄 수 반환 (점수 계산용)
}

const array<array<Cell*, BlockManager::COL>, BlockManager::ROW>& BlockManager::getCellList() const  {
	return cellList;
}

Block BlockManager::make_new_block() {
	
	if (pendingSpawn.getBlockid() != 0) {
		return std::exchange(pendingSpawn, emptyBlock);
	}
	/*
	int shape;
	int i;
	i = rand() % 100;
	BlockType type = BlockType::I;
	if (i >= stick_rate) {	//막대기 아닐 확률
		shape = (rand() % 6) + 1;		//shape에는 1~6의 값이 들어감
		type = static_cast<BlockType>(shape);
	}
	Block bl(type, ++blocks);
	*/
	return sevenBag.pop();
}

void BlockManager::appendBlock(Block& block) {
	const int row = block.getRow();
	const int col = block.getCol();
	const auto arr = block.getCells();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (isInside(row, col, i, j) && arr[i][j] != nullptr)
				cellList[row + i][col + j] = arr[i][j];
		}
	}
}



void BlockManager::removeBlock(Block& block) {
	const int row = block.getRow();
	const int col = block.getCol();
	const auto& arr = block.getCells();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (arr[i][j] != nullptr) {
				if (isInside(row, col, i, j)) {
					cellList[row + i][col + j] = nullptr;
				}
			}
		}
	}
}



bool BlockManager::canPlace(Block& block) const {
	const auto& cells = block.getCells();
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (cells[i][j] == nullptr) {
				continue;
			}

			const int row = block.getRow() + i;
			const int col = block.getCol() + j;
			if (!isInside(row, col, 0, 0) || cellList[row][col] != nullptr) {
				return false;
			}
		}
	}
	return true;
}


bool BlockManager::moveBlock(Block& currentBlock, int dr, int dc) {
	const int r = currentBlock.getRow();
	const int c = currentBlock.getCol();

	removeBlock(currentBlock);
	const auto& arr = currentBlock.getCells();

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (arr[i][j] != nullptr) {
				int nextR = r + i + dr;
				int nextC = c + j + dc;

				if (!isInside(r + dr, c + dc, i, j) || cellList[nextR][nextC] != nullptr) {
					appendBlock(currentBlock);
					return false;
				}
			}
		}
	}

	currentBlock.setPos(r + dr, c + dc);
	appendBlock(currentBlock);
	return true;
}

bool BlockManager::holdBlock(Block& currentBlock) {
	if (currentBlock.getBlockid() == holdLockedBlockId) {
		return false;
	}

	const bool firstHold = heldBlock.getBlockid() == 0;
	if (firstHold && pendingSpawn.getBlockid() == 0) {
		pendingSpawn = make_new_block();
	}
	Block incoming = firstHold ? pendingSpawn : heldBlock;
	removeBlock(currentBlock);
	if (!canPlace(incoming)) {
		appendBlock(currentBlock);
		return false;
	}

	heldBlock = currentBlock;
	heldBlock.resetForSpawn();
	currentBlock = incoming;
	if (firstHold) {
		pendingSpawn = emptyBlock;
	}
	appendBlock(currentBlock);
	holdLockedBlockId = currentBlock.getBlockid();
	return true;
}

const Block& BlockManager::getHeldBlock() const {
	return heldBlock;
}

const Block& BlockManager::getNextBlock() {
	return pendingSpawn.getBlockid() != 0 ? pendingSpawn : sevenBag.top();
}

// 기존 함수들은 한 줄로 연결
bool BlockManager::moveDown(Block& currentBlock) { return moveBlock(currentBlock, 1, 0); }
bool BlockManager::moveLeft(Block& currentBlock) { return moveBlock(currentBlock, 0, -1); }
bool BlockManager::moveRight(Block& currentBlock) { return moveBlock(currentBlock, 0, 1); }

bool BlockManager::rotateBlock(Block& currentBlock) {
	// 1. O(네모) 블록은 회전할 필요 없음
	if (currentBlock.getType() == BlockType::O) {
		return false;
	}

	const int r = currentBlock.getRow();
	const int c = currentBlock.getCol();

	// 2. 보드에서 현재 블록 지우기
	removeBlock(currentBlock);

	// 3. 기존 Block::rotate90()을 호출해 회전된 모양 미리 계산 (원본은 아직 안 바뀜!)
	auto& currentCells = currentBlock.getCells();
	auto rotatedCells = currentBlock.rotate90(currentCells);

	// 4. 회전된 모양이 벽이나 다른 블록과 부딪히는지 검사
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (rotatedCells[i][j] != nullptr) {
				if (!isInside(r, c, i, j) || cellList[r + i][c + j] != nullptr) {
					appendBlock(currentBlock);
					return false;
				}
			}
		}
	}

	// 5. 안 부딪히면 실제 블록 배열에 회전 결과 덮어쓰고 보드에 그리기
	currentCells = rotatedCells;
	appendBlock(currentBlock);
	return true;
}
