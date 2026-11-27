#include "SevenBag.h"
#include <stdlib.h>
#include <utility>

using std::exchange;
using std::swap;

SevenBag::SevenBag() {
	tryRefill();
}

SevenBag::~SevenBag() {
	clearBag();
}

void SevenBag::clearBag() {
	while (count > 0) {
		Block& block = bag[--count];
		for (const auto& row : block.getCells()) {
			for (Cell* cell : row) {
				delete cell;
			}
		}
		block = Block{};
	}
}

void SevenBag::restoreBag() {
	constexpr BlockType types[BAG_SIZE] = 
	{
		BlockType::I, 
		BlockType::O, 
		BlockType::T, 
		BlockType::J,
		BlockType::L, 
		BlockType::S, 
		BlockType::Z
	};

	clearBag();
	// 뒤에서 꺼내므로 복원할 때는 역순으로 채운다.
	for (int i = BAG_SIZE - 1; i >= 0; --i) {
		bag[count] = Block(types[i], ++lastBlockId);
		++count;
	}
}

void SevenBag::shuffleBag() {
	for (int i = count - 1; i > 0; --i) {
		const int count = i + 1;
		const int limit = RAND_MAX - RAND_MAX % count;
		int draw;
		do {
			draw = rand();
		} while (draw >= limit);
		const int j = draw % count;
		swap(bag[i], bag.at(j));
	}
}

void SevenBag::tryRefill() {
	if (count == 0) {
		restoreBag();
		shuffleBag();
	}
}

const Block& SevenBag::top() {
	tryRefill();
	return bag[count - 1];
}

Block SevenBag::pop() {
	tryRefill();
	return exchange(bag[--count], Block{});
}
