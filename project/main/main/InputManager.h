#pragma once
#include "BlockManager.h"
#include "Events.h"
#include "Inputs.h"
#include <queue>
#include <conio.h>


// 키 입력 및 처리 담당
class InputManager
{
public:
	InputManager();
	Inputs checkInput();
	Inputs processInput(BlockManager& bm, Block& currentBlock, std::queue<Events>& eventQueue);
};

