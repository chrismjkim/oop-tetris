#include "StageManager.h"

void StageManager::setLine(const int l) {
	lines = l;
}

void StageManager::addNewStage(Stage s) {
	stages[stageCount] = s;
}

Stage StageManager::getStageData(int stageNum) {
	return stages[stageNum];
}
