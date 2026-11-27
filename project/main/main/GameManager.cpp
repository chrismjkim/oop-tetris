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
		bm.resetBlocks();
		score = 0;
		lines = 0;
		while (!eventQueue.empty()) eventQueue.pop();

		current_stage = dm.showKeys();
		dm.showLayout();
		dm.showHUD(current_stage, score, lines);

		int stick_rate = sm.getStageData(current_stage).getStickRate();
		int tick = 0;

		Block currentBlock = bm.make_new_block(stick_rate);
		bm.appendBlock(currentBlock);
		dm.showCells(bm.getCellList());

		while (true) {
			bool isPlaced = false;

			// 1. 입력 이벤트를 eventQueue에 수집
			Inputs input = im.processInput(bm, currentBlock, eventQueue);
			if (input == Inputs::QUIT) {
				return;
			}
			if (input == Inputs::DOWN) {
				tick = 0;
			}
			if (input == Inputs::DROP) {
				isPlaced = true;
			}

			if (!isPlaced) {
				tick++;
				if (tick >= 20) {
					tick = 0;
					if (bm.moveDown(currentBlock)) {
						eventQueue.push(Events::ON_MOVE_BLOCK);
					}
					else {
						eventQueue.push(Events::ON_PLACE);
					}
				}
			}

			processEvents(currentBlock);

			Sleep(30);
		}
	}
}

// queue에 있는 이벤트들 순차 핸들링
void GameManager::processEvents(Block& currentBlock) {
	while (!eventQueue.empty()) {
		Events e = eventQueue.front();
		eventQueue.pop();

		switch (e)
		{
		case Events::ON_MOVE_BLOCK:
		case Events::ON_ROTATE_BLOCK:
			dm.showCells(bm.getCellList());
			break;

		case Events::ON_PLACE: {
			// 1. 먼저 꽉 찬 줄이 있는지 검사하고 삭제
			int cleared = bm.clearFullLines();
			if (cleared > 0) {
				lines += cleared;
				score += cleared * (100 + (current_stage * 10) + (rand() % 10));
				eventQueue.push(Events::ON_CLEAR_LINE);
			}

			// 2. 새 블록 생성 및 화면 갱신
			currentBlock = bm.make_new_block(sm.getStageData(current_stage).getStickRate());
			bm.appendBlock(currentBlock);
			dm.showCells(bm.getCellList());
			break;
		}

		case Events::ON_CLEAR_LINE: {
			// 스테이지 목표 라인 달성 시 다음 스테이지로 이동
			if (lines >= sm.getStageData(current_stage).getClearLine()) {
				if (current_stage < 9) {
					current_stage++;
					lines = 0;
				}
			}
			// 변경된 스탯(스테이지, 점수, 라인 수) 화면에 반영
			dm.showHUD(current_stage, score, lines);
			break;
		}

		default:
			break;
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
