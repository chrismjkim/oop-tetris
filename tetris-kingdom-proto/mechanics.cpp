#include "mechanics.h"
#include "blocks.h"
#include "board.h"

#include <algorithm>
#include <limits>

namespace tetris {

namespace {
constexpr CardInfo kCards[] = {
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
static_assert(sizeof(kCards) / sizeof(kCards[0]) == static_cast<int>(Augment::Count));
}

const CardInfo& card_info(Augment kind)
{
    return kCards[static_cast<int>(kind)];
}

int augment_count(const GameState& game, Augment kind, int shape)
{
    return static_cast<int>(std::count_if(game.augments.owned.begin(), game.augments.owned.end(),
        [=](Card card) { return card.kind == kind && card.shape == shape; }));
}

GimmickRules rules_for(int level)
{
    level = std::clamp(level, 0, 11);
    return {
        std::min(5, 1 + level),
        std::min(50, 5 * level),
        std::clamp(-10 + 10 * level, 0, 100),
        level >= 3 ? 14 : 0,
    };
}

namespace {

bool repeatable(Augment kind)
{
    return kind == Augment::Filler || kind == Augment::Bonus || kind == Augment::Steady;
}

void offer_cards(GameState& game)
{
    auto& aug = game.augments;
    const int stage = game.mode == Mode::Story ? aug.next_choice / game.chapter_goal : game.level;
    std::vector<std::vector<Card>> pool;
    for (int i = 0; i < static_cast<int>(Augment::Count); ++i) {
        const auto kind = static_cast<Augment>(i);
        const auto& info = card_info(kind);
        if (info.stage > stage || (aug.next_choice == 0 && info.shape_specific)) continue;
        std::vector<Card> shapes;
        for (int shape = info.shape_specific ? 0 : -1; shape < (info.shape_specific ? kShapeCount : 0); ++shape) {
            if (repeatable(kind) || !augment_count(game, kind, shape)) shapes.push_back({kind, shape});
        }
        if (!shapes.empty()) pool.push_back(std::move(shapes));
    }
    std::shuffle(pool.begin(), pool.end(), game.random);
    for (std::size_t i = 0; i < aug.choices.size(); ++i) {
        auto& shapes = pool[i];
        aug.choices[i] = shapes[std::uniform_int_distribution<std::size_t>(0, shapes.size() - 1)(game.random)];
    }
    aug.cursor = 0;
    game.phase = Phase::Choosing;
}

PieceSpec draw_next(GameState& game)
{
    auto& queue = game.augments.deferred_next;
    if (!queue.empty()) {
        const PieceSpec next = queue.front();
        queue.pop_front();
        return next;
    }
    auto rules = rules_for(game.level);
    for (Card card : game.augments.owned) {
        if (card.kind == Augment::SmallOnly) rules.normal_size_only[card.shape] = true;
    }
    PieceSpec spec = draw_piece(game.bag, game.random, rules);
    if (!spec.giant) return spec;
    if (augment_count(game, Augment::Shrink) && std::bernoulli_distribution(0.5)(game.random)) {
        return {spec.shape, false, -1};
    }
    if (augment_count(game, Augment::SoftGiant)) spec.reinforced = -1;
    return spec;
}

// 한 칸 내릴 때만 충돌한 고정 칸을 조사한다. 벽과 바닥은 파괴할 수 없다.
bool descent(const Board& board, const Piece& below, bool can_crush, std::optional<Point>& obstacle)
{
    if (!can_crush) return !strike_check(board, below);
    const auto cells = piece_cells(below);
    for (int i = 0; i < piece_size(below.spec); ++i) {
        const Point p = cells[i];
        if (p.x < 0 || p.x >= kWidth || p.y >= kHeight) return false;
        if (p.y >= 0 && board[p.y][p.x].durability) {
            if (obstacle) return false;
            obstacle = p;
        }
    }
    return true;
}

bool can_crush(const GameState& game)
{
    return game.current.spec.giant && !game.augments.crush_used && augment_count(game, Augment::Slam);
}

bool supported(const GameState& game)
{
    Piece below = game.current;
    ++below.origin.y;
    std::optional<Point> obstacle;
    return !descent(game.board, below, can_crush(game), obstacle);
}

void update_landing(GameState& game)
{
    Landing& landing = game.landing;
    const int y = game.current.origin.y;
    if (y > landing.lowest_y) {
        landing.lowest_y = y;
        landing.moves_left = kLockMoveLimit;
    }
    landing.grounded = supported(game);
    if (!landing.grounded) return;
    if (!landing.touched || y > landing.last_contact_y) landing.time_left_ms = kLockDelayMs;
    landing.touched = true;
    landing.last_contact_y = y;
}

void activate(GameState& game, PieceSpec spec)
{
    game.current = spawn_at_top(spec);
    game.landing = {};
    game.landing.lowest_y = game.current.origin.y;
    game.gravity_ms = 0;
    game.augments.crush_used = false;
    game.augments.active_speed = game.augments.next_speed;
    game.augments.next_speed = 1.0;
    game.phase = strike_check(game.board, game.current) ? Phase::GameOver : Phase::Playing;
    if (game.phase == Phase::Playing) update_landing(game);
}

void spawn_next(GameState& game)
{
    if (game.mode != Mode::Local) game.level = difficulty_for(game);
    if (game.mode == Mode::Story && game.total_lines >= game.chapter_goal * kChapterCount) {
        game.phase = Phase::Victory;
        return;
    }
    if (game.total_lines >= game.augments.next_choice) {
        offer_cards(game);
        return;
    }
    if (game.augments.held_return) {
        activate(game, *game.augments.held_return);
        game.augments.held_return.reset();
        return;
    }
    activate(game, game.next);
    if (game.phase == Phase::Playing) {
        game.next = draw_next(game);
    }
}

void account_cell_loss(GameState& game, ClearResult& clear, const RemovedCell& removed)
{
    const int id = removed.cell.piece_id;
    const auto entry = game.placed.find(id);
    if (entry == game.placed.end()) return;
    entry->second.intact = false;
    if (--entry->second.remaining == 0) {
        clear.destroyed.push_back({id, entry->second.spec, removed.position});
        game.placed.erase(entry);
    }
}

std::vector<int> neighbors_of(const Board& board, const std::vector<Point>& positions, int own_id)
{
    constexpr Point sides[] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
    std::vector<int> neighbors;
    for (Point p : positions) {
        for (Point side : sides) {
            const int x = p.x + side.x, y = p.y + side.y;
            if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) continue;
            const int id = board[y][x].piece_id;
            if (id && id != own_id && std::find(neighbors.begin(), neighbors.end(), id) == neighbors.end()) {
                neighbors.push_back(id);
            }
        }
    }
    return neighbors;
}

void soften(GameState& game, const std::vector<int>& neighbors)
{
    for (auto& row : game.board) {
        for (Cell& cell : row) {
            if (cell.durability != 2 ||
                std::find(neighbors.begin(), neighbors.end(), cell.piece_id) == neighbors.end()) continue;
            cell.durability = 1;
            const auto placed = game.placed.find(cell.piece_id);
            if (placed != game.placed.end()) placed->second.spec.reinforced = -1;
        }
    }
}

void queue_triggers(GameState& game, const std::vector<Trigger>& ordered)
{
    // 새 줄 제거가 만든 연쇄를 먼저 마친 뒤, 앞선 사건의 다음 증강으로 돌아간다.
    auto& stack = game.augments.pending;
    stack.insert(stack.end(), ordered.rbegin(), ordered.rend());
}

void fill_one(GameState& game)
{
    auto& aug = game.augments;
    const Point p = aug.fill_targets[aug.fill_index++];
    game.board[p.y][p.x] = {0, 1};
    refresh_void_column(game.board, aug.voids, p.x);
    aug.awaiting_clear = aug.fill_index == aug.fill_targets.size() && has_full_line(game.board);
    aug.wait_ms = aug.fill_index < aug.fill_targets.size() ? 250 : (aug.awaiting_clear ? 500 : 0);
}

bool begin_fill(GameState& game)
{
    auto& aug = game.augments;
    const auto capacity = static_cast<std::size_t>(1 + augment_count(game, Augment::Filler));
    aug.fill_targets.clear();
    for (int y = kHeight - 1; y >= 0 && aug.fill_targets.size() < capacity; --y) {
        for (int x = 0; x < kWidth && aug.fill_targets.size() < capacity; ++x) {
            if (aug.voids[y][x]) aug.fill_targets.push_back({x, y});
        }
    }
    if (aug.fill_targets.empty()) return false;
    aug.fill_index = 0;
    fill_one(game);
    return true;
}

void run_triggers(GameState& game)
{
    auto& aug = game.augments;
    while (!aug.pending.empty()) {
        Trigger trigger = std::move(aug.pending.back());
        aug.pending.pop_back();
        switch (trigger.kind) {
        case Augment::Steady:
            aug.next_speed *= 0.9;
            break;
        case Augment::Finale:
            soften(game, trigger.neighbors);
            break;
        case Augment::Repel:
            if (game.warning_id) {
                game.warning_id = 0;
                aug.repelled = true;
                game.theft_outcome = TheftOutcome::Cancelled;
            }
            break;
        case Augment::Bonus:
        case Augment::Insurance:
        case Augment::Metronome:
            if (begin_fill(game)) {
                if (--trigger.repeats > 0) aug.pending.push_back(std::move(trigger));
                game.phase = Phase::Filling;
                return;
            }
            break;
        default: break;
        }
    }
    if (aug.finish_lock) game.phase = Phase::TurnEnd;
    else spawn_next(game);
}

bool advance_fill(GameState& game, double elapsed_ms)
{
    auto& aug = game.augments;
    aug.wait_ms -= elapsed_ms;
    if (aug.wait_ms > 0) return false;
    if (aug.fill_index < aug.fill_targets.size()) {
        fill_one(game);
        return true;
    }
    if (aug.awaiting_clear) {
        aug.awaiting_clear = false;
        resolve_lines(game);
    }
    run_triggers(game);
    return true;
}

void lock_piece(GameState& game)
{
    const int id = game.next_id;
    if (!merge_block(game.board, game.current, id)) {
        game.phase = Phase::GameOver;
        return;
    }
    ++game.next_id;
    game.placed.emplace(id, PlacedPiece{game.current.spec, piece_size(game.current.spec), true});
    const auto cells = piece_cells(game.current);
    const std::vector<Point> positions(cells.begin(), cells.begin() + piece_size(game.current.spec));
    std::array<bool, kWidth> changed{};
    for (Point p : positions) changed[p.x] = true;
    for (int x = 0; x < kWidth; ++x) {
        if (changed[x]) refresh_void_column(game.board, game.augments.voids, x);
    }
    for (Card card : game.augments.owned) {
        if (card.shape != game.current.spec.shape) continue;
        if (card.kind == Augment::Softener) soften(game, neighbors_of(game.board, positions, id));
        if (card.kind == Augment::Encore && std::bernoulli_distribution(0.1)(game.random)) {
            game.augments.deferred_next.push_front(game.next);
            game.next = {card.shape, false, -1};
        }
    }
    game.used_hold = false;
    game.augments.finish_lock = true;
    resolve_lines(game);
    run_triggers(game);
}

bool lower(GameState& game)
{
    Piece candidate = game.current;
    ++candidate.origin.y;
    std::optional<Point> obstacle;
    if (!descent(game.board, candidate, can_crush(game), obstacle)) {
        update_landing(game);
        return false;
    }
    if (obstacle) {
        const Point p = *obstacle;
        ClearResult direct_loss;
        account_cell_loss(game, direct_loss, {p, game.board[p.y][p.x]});
        game.board[p.y][p.x] = {};
        refresh_void_column(game.board, game.augments.voids, p.x);
        game.augments.crush_used = true;
    }
    game.current = candidate;
    update_landing(game);
    return true;
}

std::vector<int> theft_candidates(const GameState& game, bool allow_giant)
{
    int top = 0;
    while (top < kHeight &&
        std::none_of(game.board[top].begin(), game.board[top].end(),
            [](const Cell& cell) { return cell.durability != 0; })) ++top;

    std::vector<int> candidates;
    for (int y = top; y < std::min(kHeight, top + 4); ++y) {
        for (const Cell& cell : game.board[y]) {
            if (cell.piece_id == 0) continue;
            const auto piece = game.placed.find(cell.piece_id);
            if (piece == game.placed.end() || !piece->second.intact ||
                (piece->second.spec.giant && !allow_giant)) continue;
            if (std::find(candidates.begin(), candidates.end(), cell.piece_id) == candidates.end()) {
                candidates.push_back(cell.piece_id);
            }
        }
    }
    return candidates;
}

} // namespace

Piece landing_projection(const GameState& game)
{
    Piece ghost = game.current;
    if (!can_crush(game)) {
        for (;;) {
            Piece below = ghost;
            ++below.origin.y;
            if (strike_check(game.board, below)) return ghost;
            ghost = below;
        }
    }
    Board board = game.board;
    bool crush = true;
    for (;;) {
        Piece below = ghost;
        ++below.origin.y;
        std::optional<Point> obstacle;
        if (!descent(board, below, crush, obstacle)) return ghost;
        if (obstacle) {
            const Point p = *obstacle;
            board[p.y][p.x] = {};
            crush = false;
        }
        ghost = below;
    }
}

void select_card(GameState& game, int direction)
{
    if (game.phase == Phase::Choosing) game.augments.cursor = (game.augments.cursor + direction + 3) % 3;
}

void take_card(GameState& game)
{
    if (game.phase != Phase::Choosing) return;
    auto& aug = game.augments;
    aug.owned.push_back(aug.choices[aug.cursor]);
    aug.next_choice += game.mode == Mode::Story ? game.chapter_goal / 2 : 20;
    spawn_next(game);
}

void start_run(GameState& game, Mode mode, bool debug, std::uint32_t seed)
{
    game = {};
    game.mode = mode;
    game.chapter_goal = debug ? 10 : 20;
    game.augments.next_choice = mode == Mode::Story ? 0 : 20;
    game.random.seed(seed);
    game.next = draw_piece(game.bag, game.random, rules_for(game.level));
    spawn_next(game);
}

int difficulty_for(const GameState& game)
{
    if (game.mode == Mode::Story) {
        return std::min(kChapterCount - 1, game.total_lines / game.chapter_goal);
    }
    return game.total_lines / 20;
}

int shared_difficulty(const GameState& first, const GameState& second)
{
    return (first.total_lines + second.total_lines) / 40;
}

bool try_move(GameState& game, int delta, bool rotate)
{
    if (game.phase != Phase::Playing) return false;
    Piece candidate = game.current;
    if (rotate) {
        if (candidate.spec.shape == 1 && candidate.spec.reinforced < 0) return false;
        candidate.angle = (candidate.angle + 1) % 4;
    } else {
        candidate.origin.x += delta;
    }
    if (strike_check(game.board, candidate)) return false;
    const bool was_grounded = game.landing.grounded;
    if (game.landing.touched && game.landing.moves_left > 0) --game.landing.moves_left;
    game.current = candidate;
    update_landing(game);
    if (game.landing.grounded) {
        if (game.landing.moves_left == 0) lock_piece(game);
        else if (was_grounded) game.landing.time_left_ms = kLockDelayMs;
    }
    return true;
}

bool try_hold(GameState& game)
{
    if (game.phase != Phase::Playing || game.used_hold || game.current.spec.giant) return false;
    const PieceSpec previous = game.current.spec;
    auto& aug = game.augments;
    if (game.has_hold) aug.held_return = game.hold;
    game.hold = previous;
    game.has_hold = true;
    game.used_hold = true;
    aug.hold_streak = augment_count(game, Augment::Metronome, previous.shape)
        ? (aug.last_held_shape == previous.shape ? aug.hold_streak + 1 : 1) : 0;
    aug.last_held_shape = previous.shape;
    std::vector<Trigger> triggers;
    for (Card card : aug.owned) {
        if (card.shape != previous.shape) continue;
        if (card.kind == Augment::Metronome) triggers.push_back({card.kind, aug.hold_streak});
        if (card.kind == Augment::Repel) triggers.push_back({card.kind});
    }
    aug.finish_lock = false;
    queue_triggers(game, triggers);
    run_triggers(game);
    return true;
}

void fall_step(GameState& game)
{
    if (game.phase != Phase::Playing) return;
    lower(game);
    if (game.landing.grounded && game.landing.moves_left == 0) lock_piece(game);
}

void hard_drop(GameState& game)
{
    if (game.phase != Phase::Playing) return;
    while (lower(game)) {}
    lock_piece(game);
}

bool advance_tick(GameState& game, double elapsed_ms)
{
    if (game.phase == Phase::Filling) return advance_fill(game, elapsed_ms);
    if (game.phase == Phase::TheftAnimation) {
        game.theft_flash_ms -= elapsed_ms;
        if (game.theft_flash_ms > 0) return false;
        game.stolen_cells.clear();
        if (augment_count(game, Augment::Insurance)) game.augments.pending.push_back({Augment::Insurance});
        run_triggers(game);
        return true;
    }
    if (game.phase != Phase::Playing) return false;
    double speed = rules_for(game.level).cells_per_second * game.augments.active_speed;
    if (augment_count(game, Augment::Coolant)) speed *= 0.8;
    if (augment_count(game, Augment::Slam)) speed *= 1.2;
    if (augment_count(game, Augment::Calm)) {
        int occupied = 0;
        for (const auto& row : game.board) {
            occupied += static_cast<int>(std::count_if(row.begin(), row.end(), [](Cell c) { return c.durability > 0; }));
        }
        if (occupied >= kWidth * kHeight * 7 / 10) speed *= 0.7;
    }
    const double interval = speed > 0 ? 1000.0 / speed : std::numeric_limits<double>::infinity();
    bool changed = false;
    while (elapsed_ms > 0 && game.phase == Phase::Playing) {
        if (game.landing.grounded) {
            game.landing.time_left_ms -= elapsed_ms;
            if (game.landing.time_left_ms <= 0 || game.landing.moves_left == 0) {
                lock_piece(game);
                changed = true;
            }
            break;
        }
        const double until_fall = std::max(0.0, interval - game.gravity_ms);
        if (elapsed_ms < until_fall) {
            game.gravity_ms += elapsed_ms;
            break;
        }
        elapsed_ms -= until_fall;
        game.gravity_ms = 0;
        fall_step(game);
        changed = true;
    }
    return changed;
}

ClearResult resolve_lines(GameState& game)
{
    const Board before = game.board;
    ClearResult clear = remove_full_lines(game.board);
    if (!clear.lines) return clear;
    for (const RemovedCell& cell : clear.removed) account_cell_loss(game, clear, cell);
    for (int x = 0; x < kWidth; ++x) refresh_void_column(game.board, game.augments.voids, x);
    game.total_lines += clear.lines;
    game.score += static_cast<std::int64_t>(clear.lines) * (100LL + game.level * 10LL);
    std::vector<Trigger> triggers;
    for (Card card : game.augments.owned) {
        if (card.kind == Augment::Bonus && clear.lines == 4) triggers.push_back({card.kind});
        if (card.kind != Augment::Steady && card.kind != Augment::Finale) continue;
        for (const auto& piece : clear.destroyed) {
            if (piece.spec.shape != card.shape) continue;
            Trigger trigger{card.kind};
            if (card.kind == Augment::Finale) {
                std::vector<Point> positions;
                for (const auto& cell : clear.removed) {
                    if (cell.cell.piece_id == piece.id) positions.push_back(cell.position);
                }
                trigger.neighbors = neighbors_of(before, positions, piece.id);
            }
            triggers.push_back(std::move(trigger));
        }
    }
    queue_triggers(game, triggers);
    return clear;
}

TheftOutcome resolve_theft(GameState& game, const GimmickRules& rules)
{
    game.stolen_cells.clear();
    if (game.augments.repelled) {
        game.augments.repelled = false;
        return TheftOutcome::Cancelled;
    }
    if (game.warning_id != 0) {
        const int id = game.warning_id;
        game.warning_id = 0;
        const auto target = game.placed.find(id);
        if (target == game.placed.end() || !target->second.intact) return TheftOutcome::Cancelled;

        if (augment_count(game, Augment::Tag)) {
            for (auto& row : game.board) {
                for (Cell& cell : row) {
                    if (cell.piece_id == id && cell.durability == 2) {
                        cell.durability = 1;
                        target->second.spec.reinforced = -1;
                        return TheftOutcome::Cancelled;
                    }
                }
            }
        }

        game.stolen_shape = target->second.spec.shape;
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                Cell& cell = game.board[y][x];
                if (cell.piece_id != id) continue;
                game.stolen_cells.push_back({{x, y}, cell});
                cell = {};
            }
        }
        game.placed.erase(target);
        std::array<bool, kWidth> changed{};
        for (const auto& cell : game.stolen_cells) changed[cell.position.x] = true;
        for (int x = 0; x < kWidth; ++x) {
            if (changed[x]) refresh_void_column(game.board, game.augments.voids, x);
        }
        return TheftOutcome::Stolen;
    }
    const int odds_in_thousand = rules.thief_percent * (augment_count(game, Augment::CCTV) ? 7 : 10);
    if (std::uniform_int_distribution<int>(1, 1000)(game.random) > odds_in_thousand) {
        return TheftOutcome::None;
    }
    const auto candidates = theft_candidates(game, rules.thief_can_take_giant || augment_count(game, Augment::StrongThief));
    if (candidates.empty()) return TheftOutcome::None;
    const auto slot = std::uniform_int_distribution<std::size_t>(0, candidates.size() - 1)(game.random);
    game.warning_id = candidates[slot];
    return TheftOutcome::Warned;
}

void finish_turn(GameState& game, int level)
{
    game.level = level;
    if (game.phase != Phase::TurnEnd) return;
    game.augments.finish_lock = false;
    if (game.mode == Mode::Story && game.total_lines >= game.chapter_goal * kChapterCount) {
        game.phase = Phase::Victory;
        return;
    }
    game.theft_outcome = resolve_theft(game, rules_for(game.level));
    if (game.theft_outcome == TheftOutcome::Stolen) {
        game.theft_flash_ms = kTheftFlashMs;
        game.phase = Phase::TheftAnimation;
    } else {
        spawn_next(game);
    }
}

} // namespace tetris
