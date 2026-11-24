#include "InputManager.h"
#include <conio.h>

InputManager::InputManager() {}

GameInput InputManager::checkInput() {
	if (!_kbhit()) {
		return GameInput::NONE;
	}

	const int key = _getch();
	if (key == 0 || key == 224) {
		switch (_getch()) {
		case 75: return GameInput::LEFT;
		case 77: return GameInput::RIGHT;
		case 80: return GameInput::DOWN;
		case 72: return GameInput::ROTATE;
		default: return GameInput::NONE;
		}
	}

	switch (key) {
	case ' ': return GameInput::DROP;
	case 27: return GameInput::QUIT;
	default: return GameInput::NONE;
	}
}
