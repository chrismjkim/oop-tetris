#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>

namespace tetris {

constexpr int kWidth = 10;
constexpr int kHeight = 20;
constexpr int kShapeCount = 7;
constexpr int kChapterCount = 4;
constexpr int kLockDelayMs = 500;
constexpr int kLockMoveLimit = 15;
constexpr int kTheftFlashMs = 160;

extern const char* const kShapeNames[kShapeCount];

struct Point {
    int x = 0;
    int y = 0;
};

struct PieceSpec {
    int shape = 0;
    bool giant = false;
    int reinforced = -1;
};

struct Piece {
    PieceSpec spec;
    Point origin;
    int angle = 0;
};

struct Cell {
    int piece_id = 0;
    int durability = 0;
};

using Board = std::array<std::array<Cell, kWidth>, kHeight>;
using VoidMap = std::array<std::array<bool, kWidth>, kHeight>;

struct PlacedPiece {
    PieceSpec spec;
    int remaining = 0;
    bool intact = true;
};

struct RemovedCell {
    Point position;
    Cell cell;
};

struct DestroyedPiece {
    int id;
    PieceSpec spec;
    Point final_cell;
};

struct ClearResult {
    int lines = 0;
    std::vector<RemovedCell> removed;
    std::vector<DestroyedPiece> destroyed;
};

struct GimmickRules {
    int cells_per_second;
    int thief_percent;
    int reinforce_percent;
    int giant_bag_percent;
    bool thief_can_take_giant = false;
    std::array<bool, kShapeCount> normal_size_only{};
};

struct SevenBag {
    std::array<PieceSpec, kShapeCount> pieces{};
    int next = kShapeCount;
};

enum class Mode { Story, Endless, Local };
enum class Phase { Playing, TurnEnd, TheftAnimation, Filling, Choosing, GameOver, Victory };
enum class TheftOutcome { None, Warned, Cancelled, Stolen };

enum class Augment {
    Filler, Bonus, Coolant, Slam, Insurance, Tag, CCTV, StrongThief, SoftGiant,
    Shrink, Calm, Metronome, Steady, Softener, Repel, Encore, SmallOnly, Finale, Count
};

struct Card {
    Augment kind = Augment::Filler;
    int shape = -1;
};

struct CardInfo {
    int stage;
    bool shape_specific;
    const wchar_t* name;
    const wchar_t* line1;
    const wchar_t* line2;
};

struct Trigger {
    Augment kind;
    int repeats = 1;
    std::vector<int> neighbors{};
};

struct AugmentState {
    std::vector<Card> owned;
    std::array<Card, 3> choices{};
    int cursor = 0;
    int next_choice = 0;
    int last_held_shape = -1;
    int hold_streak = 0;
    double next_speed = 1.0;
    double active_speed = 1.0;
    bool crush_used = false;
    bool repelled = false;
    std::deque<PieceSpec> deferred_next;
    std::optional<PieceSpec> held_return;
    std::vector<Trigger> pending;
    VoidMap voids{};
    std::vector<Point> fill_targets;
    std::size_t fill_index = 0;
    double wait_ms = 0;
    bool awaiting_clear = false;
    bool finish_lock = false;
};

struct Landing {
    int lowest_y = 0;
    int last_contact_y = 0;
    int moves_left = kLockMoveLimit;
    double time_left_ms = kLockDelayMs;
    bool touched = false;
    bool grounded = false;
};

struct GameState {
    Board board{};
    AugmentState augments;
    SevenBag bag;
    std::mt19937 random;
    Piece current;
    PieceSpec next;
    PieceSpec hold;
    bool has_hold = false;
    bool used_hold = false;
    Mode mode = Mode::Story;
    Phase phase = Phase::Playing;
    int chapter_goal = 20;
    int total_lines = 0;
    int level = 0;
    std::int64_t score = 0;
    Landing landing;
    double gravity_ms = 0;
    std::unordered_map<int, PlacedPiece> placed;
    int next_id = 1;
    int warning_id = 0;
    int stolen_shape = 0;
    std::vector<RemovedCell> stolen_cells;
    double theft_flash_ms = 0;
    TheftOutcome theft_outcome = TheftOutcome::None;
};

} // namespace tetris
