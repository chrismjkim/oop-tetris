#include "InputManager.h"


InputManager::InputManager() {}

Inputs InputManager::checkInput() {
	if (!_kbhit()) {
		return Inputs::NONE;
	}

	const int key = _getch();
	if (key == 0 || key == 224) {
		switch (_getch()) {
		case 75: return Inputs::LEFT;
		case 77: return Inputs::RIGHT;
		case 80: return Inputs::DOWN;
		case 72: return Inputs::ROTATE;
		default: return Inputs::NONE;
		}
	}

	switch (key) {
	case ' ': return Inputs::DROP;
	case 27: return Inputs::QUIT;
	default: return Inputs::NONE;
	}
}

Inputs InputManager::processInput(BlockManager& bm, Block& currentBlock, std::queue<Events>& eventQueue) {
	Inputs input = checkInput();

	switch (input) {
	case Inputs::LEFT:
		if (bm.moveLeft(currentBlock)) {
			eventQueue.push(Events::ON_MOVE_BLOCK);
		}
		break;
	case Inputs::RIGHT:
		if (bm.moveRight(currentBlock)) {
			eventQueue.push(Events::ON_MOVE_BLOCK);
		}
		break;
	case Inputs::DOWN:
		if (bm.moveDown(currentBlock)) {
			eventQueue.push(Events::ON_MOVE_BLOCK);
		}
		else {
			eventQueue.push(Events::ON_PLACE);
		}
		break;
	case Inputs::ROTATE:
		if (bm.rotateBlock(currentBlock)) {
			eventQueue.push(Events::ON_ROTATE_BLOCK);
		}
		break;
	case Inputs::DROP:
		while (bm.moveDown(currentBlock)) {}
		eventQueue.push(Events::ON_PLACE);
		break;
	case Inputs::QUIT:
	case Inputs::NONE:
	default:
		break;
	}

	return input;
}