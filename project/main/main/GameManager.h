#pragma once

#include "BlockManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "StageManager.h"

// 전체 게임 데이터 관리 및 진행(루프)
// 
class GameManager
{
private: 
	int score, lines = 0;
	BlockManager bm;
	DisplayManager dm;
	InputManager im;
	StageManager sm;

public:
	int current_stage = 0;
	GameManager();
	void run();
	void init();
};

