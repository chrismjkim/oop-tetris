#pragma once

class Cell {
public:
    Cell(int pieceId = 0, int durability = 0);
    int getPieceId() const;
    int getDurability() const;
    void weaken();

private:
    int pieceId;
    int durability;
};
