#pragma once
#include "Block.h"
#include <array>

constexpr int BAG_SIZE = 7;

class SevenBag
{
private:
	array<Block, BAG_SIZE> bag;
	int count = 0;
	int lastBlockId = 0;
	void clearBag();
	void tryRefill();

public:
	SevenBag();
	~SevenBag();
	SevenBag(const SevenBag&) = delete;
	// 복사 대입 생성자 방지
	SevenBag& operator=(const SevenBag&) = delete;
	void restoreBag();
	void shuffleBag();
	const Block& top();
	Block pop();
};

