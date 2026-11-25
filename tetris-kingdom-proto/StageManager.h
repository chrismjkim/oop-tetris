#pragma once

#include "board.h"
#include "BlockManager.h"
#include "Augment.h"
#include "Landing.h"
#include <cstdint>
#include <optional>
#include <random>

using namespace std;

class StageManager {
public:
    enum class Mode { Story, Endless, Local };
    enum class Phase { Playing, TurnEnd, TheftAnimation, Filling, Choosing, GameOver, Victory };
    enum class TheftOutcome { None, Warned, Cancelled, Stolen };
    static constexpr int chapterCount = 4;
    static constexpr int theftFlashMs = 160;
    void start(Mode mode, bool debug, uint32_t seed);
    void selectCard(int direction);
    void takeCard();
    bool tryMove(int delta, bool rotate);
    bool tryHold();
    void fallStep();
    void hardDrop();
    bool tick(double elapsedMs);
    void finishTurn(int level);
    int difficulty() const;
    int sharedDifficulty(const StageManager& other) const;
    static StageRules rulesFor(int level);
    Block shadow() const;
    bool finished() const;
    void endGame();

    const Board& getBoard() const;
    const Block& activeBlock() const;
    const BlockSpec& nextBlock() const;
    const BlockSpec* heldBlock() const;
    const Augment& cards() const;
    Mode getMode() const;
    Phase getPhase() const;
    int chapterGoal() const;
    int lines() const;
    int getLevel() const;
    int64_t getScore() const;
    int theftTarget() const;
    int stolenShape() const;
    const vector<RemovedCell>& stolenCells() const;
    TheftOutcome theftOutcome() const;

private:
    Board board;
    BlockManager bag;
    Block current;
    BlockSpec next;
    BlockSpec hold;
    bool hasHold = false;
    bool usedHold = false;
    optional<BlockSpec> heldReturn;
    Augment augments;
    mt19937 random;
    Landing landing;
    double gravityMs = 0;
    double activeSpeed = 1.0;
    bool crushUsed = false;
    bool finishLock = false;
    Mode mode = Mode::Story;
    Phase phase = Phase::Playing;
    int linesPerChapter = 20;
    int totalLines = 0;
    int level = 0;
    int64_t score = 0;
    int warningId = 0;
    int lastStolenShape = 0;
    vector<RemovedCell> lastStolenCells;
    double flashRemainingMs = 0;
    TheftOutcome lastTheft = TheftOutcome::None;

    void spawnNext();
    void activate(BlockSpec spec);
    bool canCrush() const;
    void updateLanding();
    bool lower();
    void lockBlock();
    LineClear resolveLines();
    TheftOutcome resolveTheft(const StageRules& rules);
    void runTriggers();
};
