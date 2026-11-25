#include "Block.h"
#include "board.h"
#include <algorithm>

using namespace std;

Block::Block(BlockSpec spec, Point origin, int angle)
    : spec(spec), origin(origin), angle(angle)
{
}

const char* Block::shapeName(int shape)
{
    static constexpr const char* names[] = {"I", "O", "T", "J", "L", "Z", "S"};
    return names[shape];
}

array<Point, 16> Block::cells() const
{
    static const Point kSpawnCells[Block::shapeCount][4] = {
        {{0, 0}, {0, 1}, {0, 2}, {0, 3}},
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 1}, {1, 1}, {1, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
        {{0, 0}, {1, 0}, {0, 1}, {0, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
    };
    const int scale = spec.getGiant() ? 2 : 1;
    const int count = spec.size();
    array<Point, 16> cells{};
    int height = 0;
    int width = 0;
    int i = 0;
    for (Point tile : kSpawnCells[spec.getShape()]) {
        for (int y = 0; y < scale; ++y) {
            for (int x = 0; x < scale; ++x) {
                const Point cell{tile.getX() * scale + x, tile.getY() * scale + y};
                cells[i++] = cell;
                width = max(width, cell.getX() + 1);
                height = max(height, cell.getY() + 1);
            }
        }
    }
    // 회전해도 각 칸의 배열 번호가 유지되어 강화 칸이 조각과 함께 돌아간다.
    for (int turn = 0; turn < angle; ++turn) {
        for (int index = 0; index < count; ++index) {
            const Point old = cells[index];
            cells[index] = {height - 1 - old.getY(), old.getX()};
        }
        swap(width, height);
    }
    for (int index = 0; index < count; ++index) {
        cells[index] = {cells[index].getX() + origin.getX(), cells[index].getY() + origin.getY()};
    }
    return cells;
}

Block Block::spawn(BlockSpec spec)
{
    Block piece{spec, {0, -3}, 0};
    const auto points = piece.cells();
    int width = 0;
    for (int i = 0; i < spec.size(); ++i) {
        width = max(width, points[i].getX() + 1);
    }
    piece.origin = {(Board::boardWidth - width) / 2, piece.origin.getY()};
    return piece;
}

const BlockSpec& Block::specification() const
{
    return spec;
}

Point Block::position() const
{
    return origin;
}

Block Block::shifted(int dx, int dy) const
{
    return {spec, {origin.getX() + dx, origin.getY() + dy}, angle};
}

Block Block::rotated() const
{
    return {spec, origin, (angle + 1) % 4};
}
