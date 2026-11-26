#pragma once

enum class Events
{
	NONE,
	ON_PLACE,
	ON_CREATE_BLOCK, // 실제 게임 진행 상에서는 블록 설치와 동시에 진행되지만, 
	// 코드 상에서 분리할 필요가 있을 것 같아 추가했습니다.
	ON_MOVE_BLOCK,
	ON_ROTATE_BLOCK,
	ON_CLEAR_LINE,
	ON_STAGE_CLEAR,
	ON_CHANGE_CELL_STATE,
	ON_CELL_ADD,
	ON_CELL_REMOVE,
};