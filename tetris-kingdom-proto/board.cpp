#include "board.h"
#include <algorithm>

using namespace std;

const Cell& Board::at(int x, int y) const
{
    return grid[y][x];
}

bool Board::filledRow(const Row& row)
{
    return all_of(row.begin(), row.end(), [](Cell cell) { return cell.getDurability() != 0; });
}

bool Board::hasFullLine() const
{
    return any_of(grid.begin(), grid.end(), filledRow);
}

void Board::refreshVoids(int x)
{
    bool above = false;
    for (int y = 0; y < Board::height; ++y) {
        voids[y][x] = above && grid[y][x].getDurability() == 0;
        above = above || grid[y][x].getDurability() != 0;
    }
    bool below = false;
    for (int y = Board::height - 1; y >= 0; --y) {
        voids[y][x] = voids[y][x] && (y == Board::height - 1 ? grid[y - 1][x].getDurability() != 0 : below);
        below = below || grid[y][x].getDurability() != 0;
    }
}

bool Board::collides(const Block& piece) const
{
    const auto cells = piece.cells();
    for (int i = 0; i < piece.specification().size(); ++i) {
        const Point cell = cells[i];
        if (cell.getX() < 0 || cell.getX() >= boardWidth || cell.getY() >= Board::height) return true;
        if (cell.getY() >= 0 && grid[cell.getY()][cell.getX()].getDurability() > 0) return true;
    }
    return false;
}

bool Board::canDescend(const Block& below, bool canCrush, optional<Point>& obstacle) const
{
    if (!canCrush) return !collides(below);
    const auto cells = below.cells();
    for (int i = 0; i < below.specification().size(); ++i) {
        const Point p = cells[i];
        if (p.getX() < 0 || p.getX() >= boardWidth || p.getY() >= Board::height) return false;
        if (p.getY() >= 0 && grid[p.getY()][p.getX()].getDurability()) {
            if (obstacle) return false;
            obstacle = p;
        }
    }
    return true;
}

bool Board::merge(const Block& piece, int pieceId)
{
    if (collides(piece)) return false;
    const auto cells = piece.cells();
    for (int i = 0; i < piece.specification().size(); ++i) {
        if (cells[i].getY() < 0) return false;
    }
    for (int i = 0; i < piece.specification().size(); ++i) {
        const Point cell = cells[i];
        grid[cell.getY()][cell.getX()] = {pieceId, i == piece.specification().getReinforced() ? 2 : 1};
    }
    return true;
}

int Board::place(const Block& piece)
{
    const int id = nextId;
    if (!merge(piece, id)) return 0;
    ++nextId;
    placed.emplace(id, PlacedBlock{piece.specification(), piece.specification().size(), true});
    const auto cells = piece.cells();
    array<bool, boardWidth> changed{};
    for (int i = 0; i < piece.specification().size(); ++i) changed[cells[i].getX()] = true;
    for (int x = 0; x < boardWidth; ++x) if (changed[x]) refreshVoids(x);
    return id;
}

void Board::recordLoss(LineClear& clear, const RemovedCell& removed)
{
    const int id = removed.getCell().getPieceId();
    const auto entry = placed.find(id);
    if (entry == placed.end()) return;
    if (entry->second.loseCell()) {
        clear.destroyed.push_back({id, entry->second.getSpec()});
        placed.erase(entry);
    }
}

LineClear Board::clearLines()
{
    LineClear result;
    array<bool, Board::height> full{};
    for (int y = 0; y < Board::height; ++y) {
        full[y] = filledRow(grid[y]);
        if (full[y]) ++result.lines;
    }
    if (result.lines == 0) return result;

    // 한 판정을 시작할 때 완성된 줄을 고정하고, 각 열을 아래부터 압축한다.
    // 강화 칸이 남은 줄은 해당 열에서 한 칸을 그대로 차지한다.
    for (int x = 0; x < boardWidth; ++x) {
        int destination = Board::height - 1;
        for (int y = Board::height - 1; y >= 0; --y) {
            Cell cell = grid[y][x];
            if (full[y] && cell.getDurability() == 1) {
                result.removed.push_back({{x, y}, cell});
                continue;
            }
            if (full[y]) cell.weaken();
            grid[destination--][x] = cell;
        }
        for (int y = destination; y >= 0; --y) grid[y][x] = {};
    }
    for (const auto& cell : result.removed) recordLoss(result, cell);
    for (int x = 0; x < boardWidth; ++x) refreshVoids(x);
    return result;
}

void Board::crushCell(Point p)
{
    LineClear loss;
    recordLoss(loss, {p, at(p.getX(), p.getY())});
    grid[p.getY()][p.getX()] = {};
    refreshVoids(p.getX());
}

vector<int> Board::neighbors(const vector<Point>& positions, int ownId) const
{
    static const Point sides[] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
    vector<int> neighbors;
    for (Point p : positions) {
        for (Point side : sides) {
            const int x = p.getX() + side.getX(), y = p.getY() + side.getY();
            if (x < 0 || y < 0 || x >= boardWidth || y >= Board::height) continue;
            const int id = grid[y][x].getPieceId();
            if (id && id != ownId && find(neighbors.begin(), neighbors.end(), id) == neighbors.end()) {
                neighbors.push_back(id);
            }
        }
    }
    return neighbors;
}

void Board::soften(const vector<int>& neighbors)
{
    for (auto& row : grid) {
        for (Cell& cell : row) {
            if (cell.getDurability() != 2 ||
                find(neighbors.begin(), neighbors.end(), cell.getPieceId()) == neighbors.end()) continue;
            cell.weaken();
            const auto entry = placed.find(cell.getPieceId());
            if (entry != placed.end()) entry->second.removeReinforcement();
        }
    }
}

vector<int> Board::theftCandidates(bool allowGiant) const
{
    int top = 0;
    while (top < Board::height &&
        none_of(grid[top].begin(), grid[top].end(),
            [](const Cell& cell) { return cell.getDurability() != 0; })) ++top;

    vector<int> candidates;
    for (int y = top; y < min(Board::height, top + 4); ++y) {
        for (const Cell& cell : grid[y]) {
            if (cell.getPieceId() == 0) continue;
            const auto piece = placed.find(cell.getPieceId());
            if (piece == placed.end() || !piece->second.getIntact() ||
                (piece->second.getSpec().getGiant() && !allowGiant)) continue;
            if (find(candidates.begin(), candidates.end(), cell.getPieceId()) == candidates.end()) {
                candidates.push_back(cell.getPieceId());
            }
        }
    }
    return candidates;
}

bool Board::isIntact(int id) const
{
    const auto it = placed.find(id);
    return it != placed.end() && it->second.getIntact();
}

int Board::shapeOf(int id) const
{
    const auto it = placed.find(id);
    return it == placed.end() ? -1 : it->second.getSpec().getShape();
}

bool Board::consumeTag(int id)
{
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.getPieceId() == id && cell.getDurability() == 2) {
                cell.weaken();
                placed.at(id).removeReinforcement();
                return true;
            }
        }
    }
    return false;
}

vector<RemovedCell> Board::steal(int id)
{
    vector<RemovedCell> stolen;
    for (int y = 0; y < Board::height; ++y) {
        for (int x = 0; x < boardWidth; ++x) {
            Cell& cell = grid[y][x];
            if (cell.getPieceId() != id) continue;
            stolen.push_back({{x, y}, cell});
            cell = {};
        }
    }
    placed.erase(id);
    array<bool, boardWidth> changed{};
    for (const auto& cell : stolen) changed[cell.getPosition().getX()] = true;
    for (int x = 0; x < boardWidth; ++x) {
        if (changed[x]) refreshVoids(x);
    }
    return stolen;
}

bool Board::isCrowded() const
{
    int occupied = 0;
    for (const auto& row : grid) {
        occupied += static_cast<int>(count_if(row.begin(), row.end(),
            [](const Cell& cell) { return cell.getDurability() > 0; }));
    }
    return occupied >= boardWidth * height * 7 / 10;
}

vector<Point> Board::fillTargets(size_t capacity) const
{
    vector<Point> targets;
    for (int y = height - 1; y >= 0 && targets.size() < capacity; --y) {
        for (int x = 0; x < boardWidth && targets.size() < capacity; ++x) {
            if (voids[y][x]) targets.emplace_back(x, y);
        }
    }
    return targets;
}

void Board::fill(Point p)
{
    grid[p.getY()][p.getX()] = {0, 1};
    refreshVoids(p.getX());
}
