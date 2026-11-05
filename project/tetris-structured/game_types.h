#pragma once

namespace tetris {

constexpr int kBoardRows = 21;
constexpr int kBoardColumns = 14;
constexpr int kPlayRows = kBoardRows - 1;
constexpr int kBlockSize = 4;
constexpr int kShapeCount = 7;
constexpr int kRotationCount = 4;
constexpr int kStageCount = 10;

struct Stage {
    int speed;       // 게임 루프 몇 회마다 자동으로 내려갈지
    int stick_rate;  // 원본의 막대 선택 기준: rand() % 100 <= stick_rate
    int clear_line;
};

constexpr Stage kStages[kStageCount] = {
    {40, 20, 20}, {38, 18, 20}, {35, 18, 20}, {30, 17, 20}, {25, 16, 20},
    {20, 14, 20}, {15, 14, 20}, {10, 13, 20}, {6, 12, 20}, {4, 11, 99999}
};

struct Piece {
    int shape = 0;
    int angle = 0;
    int x = 5;
    int y = -3;
};

struct Board {
    // 벽, 바닥, 고정된 블록만 저장하고 낙하 중인 블록은 GameState가 관리한다.
    char cells[kBoardRows][kBoardColumns] = {};
};

// 변경되는 게임 상태를 전역변수 대신 한곳에 보관한다.
struct GameState {
    Board board;
    Piece current;
    int next_shape = 0;
    int level = 0;
    int score = 0;
    int lines = 0;
};

} // namespace tetris
