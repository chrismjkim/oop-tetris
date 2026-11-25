#pragma once

#include "Block.h"
#include "Cell.h"
#include "PlacedBlock.h"
#include "LineClear.h"
#include <array>
#include <optional>
#include <unordered_map>

using namespace std;

class Board {
public:
    static constexpr int boardWidth = 10;
    static constexpr int height = 20;
    bool collides(const Block& piece) const;
    bool canDescend(const Block& below, bool crush, optional<Point>& obstacle) const;
    bool hasFullLine() const;
    int place(const Block& piece);
    LineClear clearLines();
    void crushCell(Point position);
    vector<int> neighbors(const vector<Point>& positions, int ownId) const;
    void soften(const vector<int>& ids);
    vector<int> theftCandidates(bool allowGiant) const;
    bool isIntact(int id) const;
    bool consumeTag(int id);
    vector<RemovedCell> steal(int id);
    int shapeOf(int id) const;
    const Cell& at(int x, int y) const;
    bool isCrowded() const;
    vector<Point> fillTargets(size_t capacity) const;
    void fill(Point p);

private:
    using Row = array<Cell, boardWidth>;
    array<Row, height> grid{};
    array<array<bool, boardWidth>, height> voids{};
    unordered_map<int, PlacedBlock> placed;
    int nextId = 1;

    bool merge(const Block& piece, int id);
    static bool filledRow(const Row& row);
    void refreshVoids(int x);
    void recordLoss(LineClear& clear, const RemovedCell& removed);
};
