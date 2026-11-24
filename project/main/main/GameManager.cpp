#include "GameManager.h"
#include "Block.h"
#include <Windows.h>
#include <iostream>

GameManager::GameManager() :
	bm(BlockManager()), im(InputManager()), dm(DisplayManager()), sm(StageManager()), lines(0), score(0) {

}

void GameManager::run() {
	init();
	dm.showLogo();
	current_stage = dm.showKeys();
	const Stage stage = sm.getStageData(current_stage - 1);
	dm.showGameStat(current_stage, score, lines);

	while (true) {
		Block activeBlock = bm.make_new_block(stage.getStickRate());
		if (!bm.canPlace(activeBlock)) {
			dm.showGameBoard(current_stage, bm.getCellList());
			dm.gotoxy(10, 23);
			std::cout << "Game Over";
			return;
		}

		bool locked = false;
		DWORD lastDrop = GetTickCount();
		auto lockActiveBlock = [&]() {
			bm.appendBlock(activeBlock);
			const int clearedRows = bm.clearFullRows();
			if (clearedRows > 0) {
				static const int rowScores[] = { 100, 300, 500, 800 };
				score += rowScores[clearedRows - 1];
				lines += clearedRows;
				sm.setLine(lines);
				dm.showGameStat(current_stage, score, lines);
			}
			locked = true;
		};
		auto render = [&]() {
			dm.showGameBoard(current_stage, bm.getCellList());
			dm.showBlock(activeBlock);
		};
		render();

		while (!locked) {
			bool changed = false;
			const GameInput input = im.checkInput();
			if (input == GameInput::QUIT) {
				return;
			}

			if (input == GameInput::LEFT || input == GameInput::RIGHT ||
				input == GameInput::DOWN) {
				Block movedBlock = activeBlock;
				int dx = 0;
				int dy = 0;
				if (input == GameInput::LEFT) dx = -1;
				if (input == GameInput::RIGHT) dx = 1;
				if (input == GameInput::DOWN) dy = 1;
				movedBlock.setPos(movedBlock.getX() + dx, movedBlock.getY() + dy);

				if (bm.canPlace(movedBlock)) {
					activeBlock = movedBlock;
					changed = true;
					if (input == GameInput::DOWN) {
						lastDrop = GetTickCount();
					}
				}
				else if (input == GameInput::DOWN) {
					lockActiveBlock();
				}
			}
			else if (input == GameInput::ROTATE) {
				Block rotatedBlock = activeBlock;
				const int nextAngle =
					(static_cast<int>(activeBlock.getAngle()) + 90) % 360;
				if (rotatedBlock.rotateBlock(static_cast<Angles>(nextAngle)) &&
					bm.canPlace(rotatedBlock)) {
					activeBlock = rotatedBlock;
					changed = true;
				}
			}
			else if (input == GameInput::DROP) {
				Block droppedBlock = activeBlock;
				while (true) {
					Block nextBlock = droppedBlock;
					nextBlock.setPos(nextBlock.getX(), nextBlock.getY() + 1);
					if (!bm.canPlace(nextBlock)) {
						break;
					}
					droppedBlock = nextBlock;
				}
				activeBlock = droppedBlock;
				lockActiveBlock();
				changed = true;
			}

			const DWORD now = GetTickCount();
			if (!locked && now - lastDrop >= static_cast<DWORD>(stage.getSpeed() * 10)) {
				Block fallenBlock = activeBlock;
				fallenBlock.setPos(fallenBlock.getX(), fallenBlock.getY() + 1);
				if (bm.canPlace(fallenBlock)) {
					activeBlock = fallenBlock;
					changed = true;
				}
				else {
					lockActiveBlock();
				}
				lastDrop = now;
			}

			if (changed || locked) {
				render();
			}
			Sleep(10);
		}
	}
}

void GameManager::init() {
	bm.resetBlocks();
	score = 0;
	lines = 0;
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
