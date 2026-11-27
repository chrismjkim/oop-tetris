#pragma once

#include "SevenBag.h"
#include "Block.h"
#include "BlockType.h"
#include "Cell.h"
#include <array>
#include <vector>
#include "Util.h"

using std::vector;

// 블록 생성 및 블록 개체 관리
class BlockManager
{
private:
	static inline constexpr int ROW = Util::MAXBOARDROW;
	static inline constexpr int COL = Util::MAXBOARDCOL;
	int blocks = 0;
	array<array<Cell*, COL>, ROW> cellList;
	SevenBag sevenBag = SevenBag();
	Block emptyBlock = Block();
	Block heldBlock;
	Block pendingSpawn; // 첫 홀드가 막히면 다음에 등장할 블록을 보관
	int holdLockedBlockId = 0;
	bool moveBlock(Block& currentBlock, int dr, int dc);
	
public:
	BlockManager();
	~BlockManager();
	BlockManager(const BlockManager&) = delete;
	BlockManager& operator=(const BlockManager&) = delete;
	void resetBlocks();
	bool canPlace(Block& block) const;
	Block make_new_block();
	void appendBlock(Block& block);
	void removeBlock(Block& block);
	const array<array<Cell*, BlockManager::COL>, BlockManager::ROW>& getCellList() const;
	bool isInside(const int row, const int col, const int i, const int j) const;
	bool moveDown(Block& currentBlock);
	bool moveLeft(Block& currentBlock);
	bool moveRight(Block& currentBlock);
	bool rotateBlock(Block& currentBlock);
	bool holdBlock(Block& currentBlock);
	const Block& getHeldBlock() const;
	const Block& getNextBlock();
	int clearFullLines();
};

