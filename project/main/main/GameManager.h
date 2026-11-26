#pragma once

#include <queue>
#include "BlockManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "StageManager.h"
#include "Event.h"
using std::queue;

// 전체 게임 데이터 관리 및 진행(루프)
// 
class GameManager
{
private: 
	queue<Events> eventQueue;

	int score, lines = 0;
	BlockManager bm;
	DisplayManager dm;
	InputManager im;
	StageManager sm;

	void processEvents(Block& currentBlock);

public:
	int current_stage = 0;
	GameManager();
	void run();
	void init();
};

