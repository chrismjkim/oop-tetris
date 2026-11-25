#pragma once

#include "CardInfo.h"
#include "Card.h"
#include "Trigger.h"
#include "board.h"
#include "BlockManager.h"
#include <array>
#include <deque>
#include <vector>
#include <random>

using namespace std;

class Augment {
public:
    enum class FillStep { Waiting, Placed, Clear, Done };
    static const CardInfo& info(Card::Kind kind);
    int count(Card::Kind kind, int shape = -1) const;
    void offer(int stage, mt19937& random);
    void select(int direction);
    void take(int interval);
    int nextChoice() const;
    void setFirstChoice(int lines);
    int selectedIndex() const;
    const Card& choice(int index) const;
    BlockSpec draw(BlockManager& bag, mt19937& random, StageRules rules);
    void onLock(const Block& block, Board& board, int id, mt19937& random, BlockSpec& next);
    void onHold(int shape);
    void onClear(const LineClear& clear, const Board& before);
    void onTheft();
    bool applyPending(Board& board, int& warningId);
    FillStep advanceFill(Board& board, double elapsedMs);
    double consumeNextSpeed();
    double fallSpeed(double base, const Board& board) const;
    bool consumeRepel();

private:
    vector<Card> owned;
    array<Card, 3> choices{};
    int cursor = 0;
    int nextChoiceLine = 0;
    int lastHeldShape = -1;
    int holdStreak = 0;
    double nextSpeed = 1.0;
    bool repelled = false;
    deque<BlockSpec> deferredNext;
    vector<Trigger> pending;
    vector<Point> fillTargets;
    size_t fillIndex = 0;
    double waitMs = 0;
    bool awaitingClear = false;
    void queue(const vector<Trigger>& ordered);
    void fillOne(Board& board);
    bool beginFill(Board& board);
};
