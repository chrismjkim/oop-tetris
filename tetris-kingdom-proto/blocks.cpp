#include "blocks.h"

#include <algorithm>

namespace tetris {

const char* const kShapeNames[kShapeCount] = {"I", "O", "T", "J", "L", "Z", "S"};

namespace {

// 기본 구조화판의 시작 형태를 같은 순서로 사용한다.
constexpr Point kSpawnCells[kShapeCount][4] = {
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}},
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
    {{1, 0}, {0, 1}, {1, 1}, {1, 2}},
    {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
    {{0, 0}, {1, 0}, {0, 1}, {0, 2}},
    {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
    {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
};

} // namespace

int piece_size(const PieceSpec& spec)
{
    return spec.giant ? 16 : 4;
}

std::array<Point, 16> piece_cells(const Piece& piece)
{
    const int scale = piece.spec.giant ? 2 : 1;
    const int count = piece_size(piece.spec);
    std::array<Point, 16> cells{};
    int height = 0;
    int width = 0;
    int i = 0;
    for (Point tile : kSpawnCells[piece.spec.shape]) {
        for (int y = 0; y < scale; ++y) {
            for (int x = 0; x < scale; ++x) {
                const Point cell{tile.x * scale + x, tile.y * scale + y};
                cells[i++] = cell;
                width = std::max(width, cell.x + 1);
                height = std::max(height, cell.y + 1);
            }
        }
    }
    // 회전해도 각 칸의 배열 번호가 유지되어 강화 칸이 조각과 함께 돌아간다.
    for (int turn = 0; turn < piece.angle; ++turn) {
        for (int index = 0; index < count; ++index) {
            const Point old = cells[index];
            cells[index] = {height - 1 - old.y, old.x};
        }
        std::swap(width, height);
    }
    for (int index = 0; index < count; ++index) {
        cells[index].x += piece.origin.x;
        cells[index].y += piece.origin.y;
    }
    return cells;
}

Piece spawn_at_top(PieceSpec spec)
{
    Piece piece{spec, {0, -3}, 0};
    const auto points = piece_cells(piece);
    int width = 0;
    for (int i = 0; i < piece_size(spec); ++i) {
        width = std::max(width, points[i].x + 1);
    }
    piece.origin.x = (kWidth - width) / 2;
    return piece;
}

PieceSpec draw_piece(SevenBag& bag, std::mt19937& random, const GimmickRules& rules)
{
    std::uniform_int_distribution<int> percent(1, 100);
    if (bag.next == kShapeCount) {
        for (int i = 0; i < kShapeCount; ++i) bag.pieces[i] = {i, false, -1};
        std::shuffle(bag.pieces.begin(), bag.pieces.end(), random);
        if (percent(random) <= rules.giant_bag_percent) {
            const int slot = std::uniform_int_distribution<int>(0, kShapeCount - 1)(random);
            bag.pieces[slot].giant = true;
        }
        bag.next = 0;
    }
    PieceSpec piece = bag.pieces[bag.next++];
    if (rules.normal_size_only[piece.shape]) piece.giant = false;
    if (percent(random) <= rules.reinforce_percent) {
        piece.reinforced = std::uniform_int_distribution<int>(0, piece_size(piece) - 1)(random);
    }
    return piece;
}

} // namespace tetris
