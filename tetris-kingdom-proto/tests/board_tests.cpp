#include "../blocks.h"
#include "../board.h"
#include "../mechanics.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <set>

using namespace tetris;

namespace {

int place(GameState& game, const Piece& piece)
{
    const int id = game.next_id++;
    assert(merge_block(game.board, piece, id));
    game.placed.emplace(id, PlacedPiece{piece.spec, piece_size(piece.spec), true});
    return id;
}

GimmickRules always_thief()
{
    auto rules = rules_for(1);
    rules.thief_percent = 100;
    return rules;
}

void check_geometry_and_bags()
{
    for (int shape = 0; shape < kShapeCount; ++shape) {
        Piece piece = spawn_at_top({shape, true, 0});
        assert(piece.origin.y == -3 && spawn_at_top({shape, false, -1}).origin.y == -3);
        piece.origin.y = 0; // 보드 안에서 회전별 병합 결과를 검사한다.
        for (int turn = 0; turn < 4; ++turn) {
            piece.angle = turn;
            const auto points = piece_cells(piece);
            std::set<std::pair<int, int>> unique;
            for (int i = 0; i < 16; ++i) unique.emplace(points[i].x, points[i].y);
            assert(unique.size() == 16);
            Piece centered = piece;
            centered.origin.x = 0;
            Board board{};
            assert(merge_block(board, centered, 1));
            assert(board[points[0].y][points[0].x - piece.origin.x].durability == 2);
        }
    }

    SevenBag bag;
    std::mt19937 random(721);
    const auto locked = rules_for(0);
    for (int i = 0; i < kShapeCount; ++i) {
        const PieceSpec spec = draw_piece(bag, random, locked);
        assert(!spec.giant && spec.reinforced == -1);
    }

    auto rules = rules_for(3);
    assert(rules.giant_bag_percent == 14 && rules.reinforce_percent == 20);
    rules.giant_bag_percent = 100;
    rules.reinforce_percent = 100;
    for (int batch = 0; batch < 20; ++batch) {
        std::set<int> shapes;
        int giants = 0;
        for (int i = 0; i < kShapeCount; ++i) {
            const PieceSpec spec = draw_piece(bag, random, rules);
            shapes.insert(spec.shape);
            if (spec.giant) ++giants;
            assert(spec.reinforced >= 0 && spec.reinforced < piece_size(spec));
        }
        assert(shapes.size() == kShapeCount && giants == 1);
    }

    GameState game;
    start_run(game, Mode::Story, false, 19);
    take_card(game);
    game.current = spawn_at_top({0, true, 7});
    assert(!try_hold(game));
    game.current = spawn_at_top({0, false, 2});
    assert(try_hold(game) && game.hold.reinforced == 2);
    assert(!try_hold(game));
}

void check_reinforced_lines()
{
    Board board{};
    board[16][0] = {10, 1};
    board[16][1] = {20, 1};
    for (int x = 0; x < kWidth; ++x) board[17][x] = {30 + x, x == 0 ? 2 : 1};

    const ClearResult clear = remove_full_lines(board);
    assert(clear.lines == 1 && clear.removed.size() == 9);
    assert(board[16][0].piece_id == 10 && board[17][0].piece_id == 30);
    assert(board[17][0].durability == 1);
    assert(board[17][1].piece_id == 20 && board[16][1].durability == 0);

    for (int x = 0; x < kWidth; ++x) {
        board[18][x] = {50 + x, x == 1 ? 2 : 1};
        board[19][x] = {60 + x, 1};
    }
    assert(remove_full_lines(board).lines == 2);
    assert(board[19][1].piece_id == 51 && board[19][1].durability == 1);
    assert(board[18][1].piece_id == 20);

    for (Cell& cell : board[19]) cell = {1, 2};
    const ClearResult first = remove_full_lines(board);
    assert(first.lines == 1 && first.removed.empty());
    assert(std::all_of(board[19].begin(), board[19].end(),
        [](const Cell& cell) { return cell.durability == 1; }));
    assert(remove_full_lines(board).removed.size() == kWidth);
}

void check_theft_and_destruction()
{
    GameState game;
    const auto rules = always_thief();
    const int giant = place(game, Piece{{1, true, -1}, {0, 8}, 0});
    const int target = place(game, Piece{{0, false, -1}, {5, 11}, 0});
    const int low = place(game, Piece{{0, false, -1}, {8, 13}, 0});
    assert(resolve_theft(game, rules_for(0)) == TheftOutcome::None);
    assert(resolve_theft(game, rules) == TheftOutcome::Warned);
    assert(game.warning_id == target && game.placed.count(giant) == 1);

    game.placed.at(target).intact = false;
    assert(resolve_theft(game, rules) == TheftOutcome::Cancelled);
    assert(game.warning_id == 0 && game.stolen_cells.empty());
    game.placed.at(target).intact = true;
    assert(resolve_theft(game, rules) == TheftOutcome::Warned);
    place(game, Piece{{1, false, -1}, {8, 0}, 0});
    assert(resolve_theft(game, rules) == TheftOutcome::Stolen);
    assert(game.stolen_cells.size() == 4 && game.warning_id == 0);
    assert(game.placed.count(target) == 0 && game.placed.count(low) == 1);
    assert(game.board[14][5].durability == 0);

    GameState lines;
    const int horizontal = place(lines, Piece{{0, false, -1}, {0, 19}, 1});
    place(lines, Piece{{0, false, -1}, {4, 19}, 1});
    place(lines, Piece{{1, false, -1}, {8, 18}, 0});
    const auto clear = resolve_lines(lines);
    assert(clear.lines == 1 && lines.total_lines == 1);
    assert(std::any_of(clear.destroyed.begin(), clear.destroyed.end(),
        [horizontal](const DestroyedPiece& piece) { return piece.id == horizontal; }));
    assert(lines.placed.count(horizontal) == 0);
}

void check_landing_and_progress()
{
    GameState game;
    start_run(game, Mode::Story, true, 2);
    take_card(game);
    assert(game.chapter_goal == 10);
    game.current = spawn_at_top({0, false, -1});
    game.landing = {};
    for (int i = 0; i < kHeight; ++i) fall_step(game);
    assert(game.phase == Phase::Playing && game.landing.grounded);
    assert(!advance_tick(game, 499));
    assert(try_move(game, 1, false));
    assert(game.landing.time_left_ms == kLockDelayMs && game.landing.moves_left == 14);
    for (int i = 0; i < 14; ++i) assert(try_move(game, i % 2 == 0 ? -1 : 1, false));
    assert(game.phase == Phase::TurnEnd);
    finish_turn(game, 0);
    assert(game.phase == Phase::Playing);

    game.total_lines = 39;
    assert(difficulty_for(game) == 3);
    game.total_lines = 40;
    game.phase = Phase::TurnEnd;
    finish_turn(game, difficulty_for(game));
    assert(game.phase == Phase::Victory);

    GameState first, second;
    first.total_lines = 38;
    second.total_lines = 2;
    assert(shared_difficulty(first, second) == 1);
    assert(rules_for(50).cells_per_second == 5);
    assert(rules_for(50).thief_percent == 50);
    assert(rules_for(50).reinforce_percent == 100);
}

void end_turn(GameState& game, int level)
{
    game.phase = Phase::TurnEnd;
    finish_turn(game, level);
}

void check_spawn_above_stack()
{
    GameState game;
    start_run(game, Mode::Endless, false, 1);
    place(game, Piece{{3, false, -1}, {3, 2}, 0});
    game.next = {0, false, -1};
    end_turn(game, 0);
    assert(game.phase == Phase::Playing && game.current.spec.shape == 0);
    assert(game.current.origin.x == 4 && game.current.origin.y == -3);

    GameState blocked = game;
    hard_drop(blocked);
    assert(blocked.phase == Phase::GameOver && blocked.board[1][4].durability == 0);

    assert(try_move(game, 1, false));
    hard_drop(game);
    assert(game.phase == Phase::TurnEnd && game.board[19][5].durability == 1);
}

void check_choices()
{
    std::set<Augment> seen;
    for (int seed = 0; seed < 128; ++seed) {
        GameState game;
        start_run(game, Mode::Story, seed % 2 != 0, seed);
        const int step = game.chapter_goal / 2;
        for (int i = 0; i < 8; ++i) {
            assert(game.phase == Phase::Choosing && game.augments.next_choice == i * step);
            const int bag_index = game.bag.next;
            hard_drop(game);
            assert(!try_hold(game) && !advance_tick(game, 4000) && game.bag.next == bag_index);
            std::set<Augment> unique;
            for (Card card : game.augments.choices) {
                const auto& info = card_info(card.kind);
                assert(info.stage <= i / 2 && (i || !info.shape_specific));
                assert(info.shape_specific ? card.shape >= 0 && card.shape < 7 : card.shape == -1);
                if (card.kind != Augment::Bonus && card.kind != Augment::Filler && card.kind != Augment::Steady) {
                    assert(!augment_count(game, card.kind, card.shape));
                }
                seen.insert(card.kind);
                unique.insert(card.kind);
            }
            assert(unique.size() == 3);
            select_card(game, -1);
            assert(game.augments.cursor == 2);
            select_card(game, 1);
            const Card picked = game.augments.choices[0];
            take_card(game);
            assert(game.phase == Phase::Playing && game.augments.owned.size() == static_cast<std::size_t>(i + 1));
            assert(augment_count(game, picked.kind, picked.shape));
            game.total_lines = (i + 1) * step;
            end_turn(game, difficulty_for(game));
        }
        assert(game.phase == Phase::Victory);
    }
    assert(seen.size() == static_cast<int>(Augment::Count));
    GameState first, second;
    start_run(first, Mode::Local, false, 2);
    start_run(second, Mode::Local, false, 3);
    first.total_lines = 19;
    end_turn(first, 0);
    assert(first.phase == Phase::Playing);
    first.total_lines = 20;
    end_turn(first, shared_difficulty(first, second));
    assert(first.phase == Phase::Choosing && second.phase == Phase::Playing && first.level == 0);
    const int before_fall = second.current.origin.y;
    assert(advance_tick(second, 1000) && second.current.origin.y == before_fall + 1 && !advance_tick(first, 1000));
    start_run(first, Mode::Endless, false, 4);
    first.total_lines = 41;
    end_turn(first, difficulty_for(first));
    for (int threshold : {20, 40}) {
        assert(first.phase == Phase::Choosing && first.augments.next_choice == threshold);
        take_card(first);
    }
    assert(first.phase == Phase::Playing && first.augments.owned.size() == 2);
    start_run(first, Mode::Endless, false, 4);
    assert(first.augments.owned.empty() && first.augments.next_choice == 20);
}

void sync_voids(GameState& game)
{
    for (int x = 0; x < kWidth; ++x) refresh_void_column(game.board, game.augments.voids, x);
}

void check_void_map(const GameState& game)
{
    for (int y = 0; y < kHeight; ++y) for (int x = 0; x < kWidth; ++x) {
        bool above = false, below = false;
        for (int i = 0; i < y; ++i) above |= game.board[i][x].durability > 0;
        for (int i = y + 1; i < kHeight; ++i) below |= game.board[i][x].durability > 0;
        const bool cavity = y == kHeight - 1 ? game.board[y - 1][x].durability > 0 : above && below;
        assert(game.augments.voids[y][x] == (game.board[y][x].durability == 0 && cavity));
    }
}

void check_filler_and_destruction()
{
    GameState game;
    start_run(game, Mode::Endless, false, 15);
    game.current = spawn_at_top({2, false, -1});
    game.augments.owned = {{Augment::Filler}, {Augment::Metronome, 2}, {Augment::Finale, 0},
                           {Augment::Steady, 0}, {Augment::Steady, 0}};
    for (int x = 2; x < kWidth; ++x) game.board[19][x] = {0, 1};
    game.board[19][2] = {81, 1};
    game.placed.emplace(81, PlacedPiece{{0, false, -1}, 1, false});
    game.board[18][0] = game.board[18][1] = {0, 1};
    place(game, Piece{{0, false, 3}, {2, 18}, 1});
    sync_voids(game);
    assert(try_hold(game) && game.phase == Phase::Filling);
    assert(game.board[19][0].durability == 1 && game.board[19][1].durability == 0);
    advance_tick(game, 249);
    assert(game.board[19][1].durability == 0 && !try_hold(game));
    advance_tick(game, 1);
    assert(has_full_line(game.board) && game.total_lines == 0);
    advance_tick(game, 499);
    assert(has_full_line(game.board) && game.score == 0);
    advance_tick(game, 1);
    assert(game.phase == Phase::Playing && game.total_lines == 1 && game.score == 100);
    assert(std::abs(game.augments.active_speed - 0.81) < 0.000001);
    assert(game.board[19][5].durability == 1 && !game.placed.count(81));
    check_void_map(game);
    hard_drop(game);
    finish_turn(game, 0);
    assert(game.augments.active_speed == 1);
    check_void_map(game);

    // 홀드의 충전재로 4줄이 완성되면, 뒤에 획득한 퇴치보다 보너스 연쇄가 먼저 끝난다.
    start_run(game, Mode::Endless, false, 15);
    game.current = spawn_at_top({2, false, -1});
    game.augments.owned = {{Augment::Filler}, {Augment::Filler}, {Augment::Filler},
        {Augment::Metronome, 2}, {Augment::Repel, 2}, {Augment::Bonus}, {Augment::Bonus}};
    for (int y = 16; y <= 19; ++y) {
        game.board[y].fill({0, 1});
        game.board[y][19 - y] = {};
    }
    game.board[15][3] = {0, 1};
    for (int x = 4; x < 10; ++x) game.board[14][x] = {0, 1};
    sync_voids(game);
    game.warning_id = 65;
    assert(try_hold(game));
    for (int i = 0; i < 3; ++i) advance_tick(game, 250);
    assert(game.total_lines == 0 && game.warning_id == 65);
    advance_tick(game, 500);
    assert(game.phase == Phase::Filling && game.total_lines == 4 && game.warning_id == 65);
    for (int i = 0; game.phase == Phase::Filling && i < 12; ++i) advance_tick(game, 250);
    assert(game.phase == Phase::Playing && game.warning_id == 0 && game.score == 400);
    for (int x = 4; x < 10; ++x) assert(game.board[19][x].durability == 1);
    check_void_map(game);
}

void check_placement_and_crush()
{
    GameState game;
    start_run(game, Mode::Endless, false, 8);
    game.augments.owned = {{Augment::Softener, 0}};
    for (int x = 0; x < 4; ++x) game.board[19][x] = {0, 1};
    place(game, Piece{{1, false, 2}, {4, 18}, 0});
    game.current = {{0, false, -1}, {6, 0}, 1};
    hard_drop(game);
    assert(game.total_lines == 1 && game.board[19][4].durability == 1 && !game.board[18][4].durability);
    check_void_map(game);

    for (int blockers : {1, 2}) {
        start_run(game, Mode::Endless, false, 8);
        game.augments.owned = {{Augment::Slam}, {Augment::Steady, 2}};
        game.current = spawn_at_top({1, true, -1});
        const int x = game.current.origin.x;
        game.board[14][x] = {65, 2};
        game.placed.emplace(65, PlacedPiece{{2, false, 0}, 1, false});
        if (blockers == 2) game.board[14][x + 1] = {0, 1};
        game.board[17][x] = {0, 1};
        sync_voids(game);
        const Piece ghost = landing_projection(game);
        assert(game.board[14][x].durability == 2 && !game.augments.crush_used);
        hard_drop(game);
        assert(game.current.origin.y == ghost.origin.y && ghost.origin.y == (blockers == 1 ? 13 : 10));
        assert(game.placed.count(65) == static_cast<std::size_t>(blockers - 1));
        assert(game.augments.next_speed == 1 && game.total_lines == 0);
        check_void_map(game);
    }
}

void check_generation_and_encore()
{
    GameState game;
    int shrunk = 0, echoed = 0;
    for (int seed = 0; seed < 300; ++seed) {
        for (Augment kind : {Augment::Shrink, Augment::SmallOnly, Augment::SoftGiant}) {
            start_run(game, Mode::Local, false, seed);
            game.augments.owned = {{kind, kind == Augment::SmallOnly ? 3 : -1}};
            game.bag.next = 0;
            const int shape = seed % 2 ? 2 : 3;
            game.bag.pieces[0] = {shape, true, -1};
            end_turn(game, 11);
            assert(game.next.shape == shape);
            if (kind == Augment::SoftGiant) assert(game.next.giant && game.next.reinforced == -1);
            if (kind == Augment::SmallOnly) assert(game.next.giant == (shape != 3));
            if (!game.next.giant) {
                if (kind == Augment::Shrink) {
                    assert(game.next.reinforced == -1);
                    ++shrunk;
                } else assert(game.next.reinforced >= 0 && game.next.reinforced < 4);
            } else if (kind != Augment::SoftGiant) assert(game.next.reinforced >= 0);
        }
        start_run(game, Mode::Endless, false, seed);
        game.current = spawn_at_top({0, false, -1});
        game.next = {2, false, 3};
        game.augments.owned = {{Augment::Encore, 0}};
        const auto index = game.bag.next;
        hard_drop(game);
        const bool encore = !game.augments.deferred_next.empty();
        finish_turn(game, 0);
        if (encore) {
            ++echoed;
            assert(game.current.spec.shape == 0 && game.next.shape == 2 && game.next.reinforced == 3);
            assert(game.bag.next == index && game.augments.deferred_next.empty());
        } else assert(game.current.spec.shape == 2);
    }
    assert(shrunk > 100 && shrunk < 200 && echoed > 10 && echoed < 55);
}

void check_speed_and_thief_augments()
{
    GameState game;
    for (int occupied : {139, 140}) {
        start_run(game, Mode::Endless, false, 9);
        game.augments.owned = {{Augment::Coolant}, {Augment::Calm}, {Augment::Slam}};
        for (int i = 0; i < occupied; ++i) game.board[19 - i / 10][i % 10] = {0, 1};
        const int interval = occupied == 139 ? 1042 : 1489;
        assert(!advance_tick(game, interval - 1) && advance_tick(game, 1));
    }
    start_run(game, Mode::Endless, false, 42);
    const int target = place(game, Piece{{1, true, 0}, {0, 12}, 0});
    game.augments.owned = {{Augment::StrongThief}, {Augment::Tag}, {Augment::Insurance}};
    assert(resolve_theft(game, always_thief()) == TheftOutcome::Warned && game.warning_id == target);
    assert(resolve_theft(game, always_thief()) == TheftOutcome::Cancelled);
    assert(game.board[12][0].durability == 1 && game.placed.at(target).intact);
    game.warning_id = target;
    game.board[18][9] = {0, 1};
    sync_voids(game);
    end_turn(game, 0);
    assert(game.phase == Phase::TheftAnimation && !game.placed.count(target));
    advance_tick(game, kTheftFlashMs);
    assert(game.phase == Phase::Filling && game.board[19][9].durability == 1);
    advance_tick(game, 1);
    assert(game.phase == Phase::Playing);
    check_void_map(game);

    start_run(game, Mode::Endless, false, 42);
    place(game, Piece{{1, false, -1}, {0, 12}, 0});
    game.augments.owned = {{Augment::CCTV}};
    bool observed_reduction = false;
    for (int seed = 0; seed < 1000; ++seed) {
        game.random.seed(seed);
        auto preview = game.random;
        const int roll = std::uniform_int_distribution<int>(1,1000)(preview);
        game.warning_id = 0;
        assert((resolve_theft(game, rules_for(1)) == TheftOutcome::Warned) == (roll <= 35));
        observed_reduction |= roll > 35 && roll <= 50;
    }
    assert(observed_reduction);
}

} // namespace

int main()
{
    check_geometry_and_bags();
    check_spawn_above_stack();
    check_reinforced_lines();
    check_theft_and_destruction();
    check_landing_and_progress();
    check_choices();
    check_filler_and_destruction();
    check_placement_and_crush();
    check_generation_and_encore();
    check_speed_and_thief_augments();
    std::puts("PASS: giant geometry and bag, reinforcement, theft, destruction, locking and progression");
    std::puts("PASS: all 18 augments, story/endless/local choices, stacking, animated filler and recursive triggers");
}
