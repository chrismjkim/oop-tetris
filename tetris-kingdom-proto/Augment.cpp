#include "Augment.h"
#include <algorithm>
#include <utility>

using namespace std;

const CardInfo& Augment::info(Card::Kind kind)
{
    static const CardInfo kCards[] = {
        {0, false, L"충전재 증량", L"충전재 발동마다", L"공극을 1칸 더 채움"},
        {0, false, L"보너스 지급", L"한 번에 4줄 제거 시", L"충전재 1회 발동"},
        {0, false, L"냉각제", L"자동 낙하 속도", L"20% 감소"},
        {3, false, L"내리꽂기", L"거대 조각 아래 파괴", L"자동 낙하 속도 +20%"},
        {1, false, L"손해보험", L"도둑이 훔쳐 가면", L"충전재 1회 발동"},
        {2, false, L"도난 방지 태그", L"강화 대상 절도 취소", L"대상의 강화 해제"},
        {1, false, L"CCTV 설치", L"도둑 출현 확률", L"기존의 70%로 감소"},
        {3, false, L"힘 센 도둑", L"도둑의 절도 대상에", L"거대 조각도 포함"},
        {3, false, L"물렁물렁", L"거대 조각 생성 시", L"강화 칸이 사라짐"},
        {3, false, L"축소 수술", L"거대 조각 50% 확률", L"축소 및 강화 해제"},
        {0, false, L"패닉하지 않기", L"보드 70% 이상 차면", L"낙하 속도 30% 감소"},
        {0, true, L"메트로놈", L"연속 홀드 횟수만큼", L"충전재 발동"},
        {0, true, L"천천히 꾸준히", L"완전 파괴 시 다음", L"조각 낙하 속도 -10%"},
        {2, true, L"연화제", L"설치 직후 인접 조각", L"강화 해제"},
        {1, true, L"몽둥이를 들어라", L"홀드하면 도둑의", L"예고된 절도 취소"},
        {0, true, L"앵콜", L"설치 시 10% 확률로", L"다음에 같은 형태"},
        {3, true, L"도토리 키재기", L"이 형태의 조각은", L"일반 크기로 등장"},
        {2, true, L"갈 때도 예술로", L"완전 파괴 시 인접", L"조각의 강화 해제"}
    };
    static_assert(sizeof(kCards) / sizeof(kCards[0]) == static_cast<int>(Card::Kind::Count));
    return kCards[static_cast<int>(kind)];
}

int Augment::count(Card::Kind kind, int shape) const
{
    return static_cast<int>(count_if(owned.begin(), owned.end(),
        [=](Card card) { return card.getKind() == kind && card.getShape() == shape; }));
}

void Augment::offer(int stage, mt19937& random)
{
    vector<vector<Card>> pool;
    for (int i = 0; i < static_cast<int>(Card::Kind::Count); ++i) {
        const auto kind = static_cast<Card::Kind>(i);
        const auto& info = Augment::info(kind);
        if (info.getStage() > stage || (nextChoiceLine == 0 && info.getShapeSpecific())) continue;
        vector<Card> shapes;
        const int firstShape = info.getShapeSpecific() ? 0 : -1;
        const int endShape = info.getShapeSpecific() ? Block::shapeCount : 0;
        for (int shape = firstShape; shape < endShape; ++shape) {
            Card candidate(kind, shape);
            if (candidate.repeatable() || !count(kind, shape)) shapes.push_back(candidate);
        }
        if (!shapes.empty()) pool.push_back(move(shapes));
    }
    shuffle(pool.begin(), pool.end(), random);
    for (size_t i = 0; i < choices.size(); ++i) {
        auto& shapes = pool[i];
        choices[i] = shapes[uniform_int_distribution<size_t>(0, shapes.size() - 1)(random)];
    }
    cursor = 0;
}

void Augment::select(int direction)
{
    cursor = (cursor + direction + 3) % 3;
}

void Augment::take(int interval)
{
    owned.push_back(choices[cursor]);
    nextChoiceLine += interval;
}

int Augment::nextChoice() const
{
    return nextChoiceLine;
}

int Augment::selectedIndex() const
{
    return cursor;
}

const Card& Augment::choice(int index) const
{
    return choices[index];
}

void Augment::setFirstChoice(int lines)
{
    nextChoiceLine = lines;
}

BlockSpec Augment::draw(BlockManager& bag, mt19937& random, StageRules rules)
{
    if (!deferredNext.empty()) {
        const BlockSpec next = deferredNext.front();
        deferredNext.pop_front();
        return next;
    }
    for (Card card : owned) {
        if (card.getKind() == Card::Kind::SmallOnly) rules.requireNormalSize(card.getShape());
    }
    BlockSpec spec = bag.draw(random, rules);
    if (!spec.getGiant()) return spec;
    if (count(Card::Kind::Shrink) && bernoulli_distribution(0.5)(random)) {
        return {spec.getShape(), false, -1};
    }
    if (count(Card::Kind::SoftGiant)) spec.setReinforcedCell(-1);
    return spec;
}

void Augment::onLock(const Block& block, Board& board, int id, mt19937& random, BlockSpec& next)
{
    for (Card card : owned) {
        if (card.getShape() != block.specification().getShape()) continue;
        if (card.getKind() == Card::Kind::Softener) {
            const auto cells = block.cells();
            const vector<Point> positions(cells.begin(), cells.begin() + block.specification().size());
            board.soften(board.neighbors(positions, id));
        }
        if (card.getKind() == Card::Kind::Encore && bernoulli_distribution(0.1)(random)) {
            deferredNext.push_front(next);
            next = {card.getShape(), false, -1};
        }
    }
}

void Augment::onHold(int shape)
{
    holdStreak = count(Card::Kind::Metronome, shape)
        ? (lastHeldShape == shape ? holdStreak + 1 : 1) : 0;
    lastHeldShape = shape;
    vector<Trigger> triggers;
    for (Card card : owned) {
        if (card.getShape() != shape) continue;
        if (card.getKind() == Card::Kind::Metronome) triggers.push_back({card.getKind(), holdStreak});
        if (card.getKind() == Card::Kind::Repel) triggers.push_back({card.getKind()});
    }
    queue(triggers);
}

void Augment::onClear(const LineClear& clear, const Board& before)
{
    vector<Trigger> triggers;
    for (Card card : owned) {
        if (card.getKind() == Card::Kind::Bonus && clear.lineCount() == 4) triggers.push_back({card.getKind()});
        if (card.getKind() != Card::Kind::Steady && card.getKind() != Card::Kind::Finale) continue;
        for (const auto& piece : clear.destroyedBlocks()) {
            if (piece.getSpec().getShape() != card.getShape()) continue;
            Trigger trigger{card.getKind()};
            if (card.getKind() == Card::Kind::Finale) {
                vector<Point> positions;
                for (const auto& cell : clear.removedCells()) {
                    if (cell.getCell().getPieceId() == piece.getId()) positions.push_back(cell.getPosition());
                }
                trigger = Trigger(card.getKind(), 1, before.neighbors(positions, piece.getId()));
            }
            triggers.push_back(move(trigger));
        }
    }
    queue(triggers);
}

void Augment::onTheft()
{
    if (count(Card::Kind::Insurance)) pending.emplace_back(Card::Kind::Insurance);
}

void Augment::queue(const vector<Trigger>& ordered)
{
    // 같은 사건은 획득 순서로, 새 연쇄는 대기 중인 효과보다 먼저 실행한다.
    pending.insert(pending.end(), ordered.rbegin(), ordered.rend());
}

bool Augment::applyPending(Board& board, int& warningId)
{
    while (!pending.empty()) {
        Trigger trigger = move(pending.back());
        pending.pop_back();
        switch (trigger.getKind()) {
        case Card::Kind::Steady:
            nextSpeed *= 0.9;
            break;
        case Card::Kind::Finale:
            board.soften(trigger.getNeighbors());
            break;
        case Card::Kind::Repel:
            if (warningId) {
                warningId = 0;
                repelled = true;
            }
            break;
        case Card::Kind::Bonus:
        case Card::Kind::Insurance:
        case Card::Kind::Metronome:
            if (beginFill(board)) {
                if (trigger.repeat()) pending.push_back(move(trigger));
                return true;
            }
            break;
        default: break;
        }
    }
    return false;
}

void Augment::fillOne(Board& board)
{
    const Point p = fillTargets[fillIndex++];
    board.fill(p);
    awaitingClear = fillIndex == fillTargets.size() && board.hasFullLine();
    waitMs = fillIndex < fillTargets.size() ? 250 : (awaitingClear ? 500 : 0);
}

bool Augment::beginFill(Board& board)
{
    fillTargets = board.fillTargets(1 + count(Card::Kind::Filler));
    if (fillTargets.empty()) return false;
    fillIndex = 0;
    fillOne(board);
    return true;
}

Augment::FillStep Augment::advanceFill(Board& board, double elapsedMs)
{
    waitMs -= elapsedMs;
    if (waitMs > 0) return FillStep::Waiting;
    if (fillIndex < fillTargets.size()) {
        fillOne(board);
        return FillStep::Placed;
    }
    if (awaitingClear) {
        awaitingClear = false;
        return FillStep::Clear;
    }
    return FillStep::Done;
}

bool Augment::consumeRepel()
{
    return exchange(repelled, false);
}

double Augment::consumeNextSpeed()
{
    return exchange(nextSpeed, 1.0);
}

double Augment::fallSpeed(double base, const Board& board) const
{
    if (count(Card::Kind::Coolant)) base *= 0.8;
    if (count(Card::Kind::Slam)) base *= 1.2;
    if (count(Card::Kind::Calm) && board.isCrowded()) base *= 0.7;
    return base;
}
