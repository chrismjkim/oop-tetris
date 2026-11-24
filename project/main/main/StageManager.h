#pragma once
#include "Stage.h"

// 현재 스테이지 상태, 스테이지 관리
class StageManager
{
private: 
	int lines = 0;
	bool isGameOver = false;
	int stageCount = 0;
	Stage stages[10];
public:
	StageManager()=default;
	void setLine(const int l);
	void addNewStage(Stage s);
	Stage getStageData(int stageNum);
};

