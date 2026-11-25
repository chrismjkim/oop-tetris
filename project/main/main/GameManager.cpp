#include "GameManager.h"
#include "Block.h"
#include <Windows.h>


GameManager::GameManager() :
	bm(), im(), dm(), sm(), lines(0), score(0) {

}

void GameManager::run() {
	init();
	dm.showLogo();
	while (true) {
		current_stage = dm.showKeys();
		dm.showGameBoard(current_stage, bm.getCellList());
		dm.showGameStat(current_stage, score, lines);
		int stick_rate = sm.getStageData(current_stage).getStickRate();
		Block newBlock = bm.make_new_block(stick_rate);
		bm.appendBlock(newBlock);
		dm.showBlock(newBlock);
		while (true) {
			//auto input = im.checkInput();
			Sleep(100);

			dm.eraseBlock(newBlock);

			if (bm.downBlock(newBlock)) {
				dm.showBlock(newBlock);
			}

		}
	}
}

void GameManager::init() {
	bm.resetBlocks();
	sm.setLine(0);
	dm.resetab_xy();
	current_stage = 0;
	sm.addNewStage(Stage(40, 20, 20));
	sm.addNewStage(Stage(38, 18, 20));
	sm.addNewStage(Stage(35, 18, 20));
	sm.addNewStage(Stage(30, 17, 20));
	sm.addNewStage(Stage(25, 16, 20));
	sm.addNewStage(Stage(20, 14, 20));
	sm.addNewStage(Stage(15, 14, 20));
	sm.addNewStage(Stage(10, 13, 20));
	sm.addNewStage(Stage(6, 12, 20));
	sm.addNewStage(Stage(4, 11, 99999));
}
