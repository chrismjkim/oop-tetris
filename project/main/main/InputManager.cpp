#include "InputManager.h"
#include "KeyCode.h"

InputManager::InputManager() {}

Inputs InputManager::checkInput() {
	if (!_kbhit()) {
		return Inputs::NONE;
	}

	const int key = _getch();
	if (key == 0 || key == 224) {
		switch (static_cast<KeyCode>(_getch())) {
		case KeyCode::ARROW_LEFT: return Inputs::LEFT;
		case KeyCode::ARROW_RIGHT: return Inputs::RIGHT;
		case KeyCode::ARROW_DOWN: return Inputs::DOWN;
		case KeyCode::ARROW_UP: return Inputs::ROTATE;
		default: return Inputs::NONE;
		}
	}

	switch (static_cast<KeyCode>(key)) {
	case KeyCode::SPACE: return Inputs::DROP;
	case KeyCode::C: return Inputs::HOLD;
	case KeyCode::ESC: return Inputs::QUIT;
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
	case Inputs::HOLD:
		// 첫 홀드에 필요한 스테이지별 생성 확률은 GameManager에서 전달한다.
		return input;
	case Inputs::QUIT:
	case Inputs::NONE:
	default:
		break;
	}

	return input;
}
