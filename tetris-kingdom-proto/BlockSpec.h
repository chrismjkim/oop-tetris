#pragma once

class BlockSpec {
public:
    BlockSpec(int shape = 0, bool giant = false, int reinforced = -1);
    int getShape() const;
    bool getGiant() const;
    int getReinforced() const;
    int size() const;
    void setGiant(bool value);
    void setReinforcedCell(int index);

private:
    int shape;
    bool giant;
    int reinforced;
};
