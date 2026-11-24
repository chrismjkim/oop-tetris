#pragma once

enum class GameInput
{
	NONE,
	LEFT,
	RIGHT,
	DOWN,
	DROP,
	ROTATE,
	QUIT
};

// 키 입력 및 처리 담당
class InputManager
{
public:
	InputManager();
	GameInput checkInput();
};

