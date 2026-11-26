#pragma once
#include "Stage.h"

// 스테이지별 기믹 관리
class StageManager
{
	// NOTE: 실제 아래 구현은 아예 바꿔야 할 것 같습니다.
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

