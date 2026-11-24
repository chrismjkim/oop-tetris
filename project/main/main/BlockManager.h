#pragma once

#include "Block.h"
#include "BlockType.h"
#include <array>
#include <vector>

using std::vector;

// 블록 생성 및 블록 개체 관리
class BlockManager
{
private:
	static inline constexpr int ROW = 20;
	static inline constexpr int COL = 10;
	vector<Block> blockList;
	array<array<int, COL>, ROW> cellList;
public:
	BlockManager();
	void resetBlocks();
	Block make_new_block(int stick_rate);
	void appendBlock(Block& block);
	int clearFullRows();
	bool canPlace(const Block& block) const;
	const array<array<int, BlockManager::COL>, BlockManager::ROW>& getCellList() const;
	const vector<Block>& getBlockList() const;
};

