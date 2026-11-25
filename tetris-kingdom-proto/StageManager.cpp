#include "StageManager.h"
#include <algorithm>
#include <limits>

using namespace std;

StageRules StageManager::rulesFor(int level)
{
    level = clamp(level, 0, 11);
    return {
        min(5, 1 + level),
        min(50, 5 * level),
        clamp(-10 + 10 * level, 0, 100),
        level >= 3 ? 14 : 0,
    };
}

void StageManager::start(Mode chosenMode, bool debug, uint32_t seed)
{
    *this = StageManager{};
    mode = chosenMode;
    linesPerChapter = debug ? 10 : 20;
    augments.setFirstChoice(mode == Mode::Story ? 0 : 20);
    random.seed(seed);
    next = bag.draw(random, rulesFor(level));
    spawnNext();
}

int StageManager::difficulty() const
{
    if (mode == Mode::Story) {
        return min(StageManager::chapterCount - 1, totalLines / linesPerChapter);
    }
    return totalLines / 20;
}

int StageManager::sharedDifficulty(const StageManager& other) const
{
    return (totalLines + other.totalLines) / 40;
}

void StageManager::selectCard(int direction)
{
    if (phase == Phase::Choosing) augments.select(direction);
}

void StageManager::takeCard()
{
    if (phase != Phase::Choosing) return;
    augments.take(mode == Mode::Story ? linesPerChapter / 2 : 20);
    spawnNext();
}

void StageManager::spawnNext()
{
    if (mode != Mode::Local) level = difficulty();
    if (mode == Mode::Story && totalLines >= linesPerChapter * StageManager::chapterCount) {
        phase = Phase::Victory;
        return;
    }
    if (totalLines >= augments.nextChoice()) {
        const int stage = mode == Mode::Story ? augments.nextChoice() / linesPerChapter : level;
        augments.offer(stage, random);
        phase = Phase::Choosing;
        return;
    }
    if (heldReturn) {
        activate(*heldReturn);
        heldReturn.reset();
        return;
    }
    activate(next);
    if (phase == Phase::Playing) {
        next = augments.draw(bag, random, rulesFor(level));
    }
}

void StageManager::activate(BlockSpec spec)
{
    current = Block::spawn(spec);
    landing.reset(current.position().getY());
    gravityMs = 0;
    crushUsed = false;
    activeSpeed = augments.consumeNextSpeed();
    phase = board.collides(current) ? Phase::GameOver : Phase::Playing;
    if (phase == Phase::Playing) updateLanding();
}

bool StageManager::canCrush() const
{
    return current.specification().getGiant() && !crushUsed && augments.count(Card::Kind::Slam);
}

void StageManager::updateLanding()
{
    optional<Point> obstacle;
    const bool supported = !board.canDescend(current.shifted(0, 1), canCrush(), obstacle);
    landing.update(current.position().getY(), supported);
}

Block StageManager::shadow() const
{
    Block ghost = current;
    if (!canCrush()) {
        for (;;) {
            Block below = ghost.shifted(0, 1);
            if (board.collides(below)) return ghost;
            ghost = below;
        }
    }
    Board projection = board;
    bool crush = true;
    for (;;) {
        Block below = ghost.shifted(0, 1);
        optional<Point> obstacle;
        if (!projection.canDescend(below, crush, obstacle)) return ghost;
        if (obstacle) {
            projection.crushCell(*obstacle);
            crush = false;
        }
        ghost = below;
    }
}

bool StageManager::tryMove(int delta, bool rotate)
{
    if (phase != Phase::Playing) return false;
    Block candidate = current;
    if (rotate) {
        if (candidate.specification().getShape() == 1 && candidate.specification().getReinforced() < 0) return false;
        candidate = candidate.rotated();
    } else {
        candidate = candidate.shifted(delta, 0);
    }
    if (board.collides(candidate)) return false;
    const bool was_grounded = landing.groundedNow();
    landing.useMove();
    current = candidate;
    updateLanding();
    if (landing.groundedNow()) {
        if (landing.outOfMoves()) lockBlock();
        else if (was_grounded) landing.resetTimer();
    }
    return true;
}

bool StageManager::lower()
{
    const Block candidate = current.shifted(0, 1);
    optional<Point> obstacle;
    if (!board.canDescend(candidate, canCrush(), obstacle)) {
        updateLanding();
        return false;
    }
    if (obstacle) {
        board.crushCell(*obstacle);
        crushUsed = true;
    }
    current = candidate;
    updateLanding();
    return true;
}

void StageManager::fallStep()
{
    if (phase != Phase::Playing) return;
    lower();
    if (landing.groundedNow() && landing.outOfMoves()) lockBlock();
}

void StageManager::hardDrop()
{
    if (phase != Phase::Playing) return;
    while (lower()) {}
    lockBlock();
}

bool StageManager::tryHold()
{
    if (phase != Phase::Playing || usedHold || current.specification().getGiant()) return false;
    if (hasHold) heldReturn = hold;
    hold = current.specification();
    hasHold = true;
    usedHold = true;
    augments.onHold(hold.getShape());
    finishLock = false;
    runTriggers();
    return true;
}

void StageManager::lockBlock()
{
    const int id = board.place(current);
    if (id == 0) {
        phase = Phase::GameOver;
        return;
    }
    augments.onLock(current, board, id, random, next);
    usedHold = false;
    finishLock = true;
    resolveLines();
    runTriggers();
}

void StageManager::runTriggers()
{
    const int previousTarget = warningId;
    const bool filling = augments.applyPending(board, warningId);
    if (previousTarget && !warningId) lastTheft = TheftOutcome::Cancelled;
    if (filling) phase = Phase::Filling;
    else if (finishLock) phase = Phase::TurnEnd;
    else spawnNext();
}

LineClear StageManager::resolveLines()
{
    if (!board.hasFullLine()) return {};
    const Board before = board;
    LineClear clear = board.clearLines();
    totalLines += clear.lineCount();
    score += static_cast<int64_t>(clear.lineCount()) * (100LL + level * 10LL);
    augments.onClear(clear, before);
    return clear;
}

bool StageManager::tick(double elapsedMs)
{
    if (phase == Phase::Filling) {
        const auto step = augments.advanceFill(board, elapsedMs);
        if (step == Augment::FillStep::Waiting) return false;
        if (step == Augment::FillStep::Placed) return true;
        if (step == Augment::FillStep::Clear) resolveLines();
        runTriggers();
        return true;
    }
    if (phase == Phase::TheftAnimation) {
        flashRemainingMs -= elapsedMs;
        if (flashRemainingMs > 0) return false;
        lastStolenCells.clear();
        augments.onTheft();
        runTriggers();
        return true;
    }
    if (phase != Phase::Playing) return false;
    const double speed = augments.fallSpeed(rulesFor(level).getCellsPerSecond() * activeSpeed, board);
    const double interval = speed > 0 ? 1000.0 / speed : numeric_limits<double>::infinity();
    bool changed = false;
    while (elapsedMs > 0 && phase == Phase::Playing) {
        if (landing.groundedNow()) {
            landing.wait(elapsedMs);
            if (landing.expired()) {
                lockBlock();
                changed = true;
            }
            break;
        }
        const double until_fall = max(0.0, interval - gravityMs);
        if (elapsedMs < until_fall) {
            gravityMs += elapsedMs;
            break;
        }
        elapsedMs -= until_fall;
        gravityMs = 0;
        fallStep();
        changed = true;
    }
    return changed;
}

StageManager::TheftOutcome StageManager::resolveTheft(const StageRules& rules)
{
    lastStolenCells.clear();
    if (augments.consumeRepel()) return TheftOutcome::Cancelled;
    if (warningId != 0) {
        const int id = warningId;
        warningId = 0;
        if (!board.isIntact(id)) return TheftOutcome::Cancelled;
        if (augments.count(Card::Kind::Tag) && board.consumeTag(id)) return TheftOutcome::Cancelled;
        lastStolenShape = board.shapeOf(id);
        lastStolenCells = board.steal(id);
        return TheftOutcome::Stolen;
    }
    const int odds_in_thousand = rules.getThiefPercent() * (augments.count(Card::Kind::CCTV) ? 7 : 10);
    if (uniform_int_distribution<int>(1, 1000)(random) > odds_in_thousand) {
        return TheftOutcome::None;
    }
    const auto candidates = board.theftCandidates(rules.getThiefCanTakeGiant() || augments.count(Card::Kind::StrongThief));
    if (candidates.empty()) return TheftOutcome::None;
    const auto slot = uniform_int_distribution<size_t>(0, candidates.size() - 1)(random);
    warningId = candidates[slot];
    return TheftOutcome::Warned;
}

void StageManager::finishTurn(int newLevel)
{
    level = newLevel;
    if (phase != Phase::TurnEnd) return;
    finishLock = false;
    if (mode == Mode::Story && totalLines >= linesPerChapter * StageManager::chapterCount) {
        phase = Phase::Victory;
        return;
    }
    lastTheft = resolveTheft(StageManager::rulesFor(level));
    if (lastTheft == TheftOutcome::Stolen) {
        flashRemainingMs = theftFlashMs;
        phase = Phase::TheftAnimation;
    } else {
        spawnNext();
    }
}

bool StageManager::finished() const
{
    return phase == Phase::GameOver || phase == Phase::Victory;
}

const Board& StageManager::getBoard() const
{
    return board;
}

const Block& StageManager::activeBlock() const
{
    return current;
}

const BlockSpec& StageManager::nextBlock() const
{
    return next;
}

const BlockSpec* StageManager::heldBlock() const
{
    return hasHold ? &hold : nullptr;
}

const Augment& StageManager::cards() const
{
    return augments;
}

StageManager::Mode StageManager::getMode() const
{
    return mode;
}

StageManager::Phase StageManager::getPhase() const
{
    return phase;
}

int StageManager::chapterGoal() const
{
    return linesPerChapter;
}

int StageManager::lines() const
{
    return totalLines;
}

int StageManager::getLevel() const
{
    return level;
}

int64_t StageManager::getScore() const
{
    return score;
}

int StageManager::theftTarget() const
{
    return warningId;
}

int StageManager::stolenShape() const
{
    return lastStolenShape;
}

const vector<RemovedCell>& StageManager::stolenCells() const
{
    return lastStolenCells;
}

StageManager::TheftOutcome StageManager::theftOutcome() const
{
    return lastTheft;
}

void StageManager::endGame()
{
    phase = Phase::GameOver;
}
