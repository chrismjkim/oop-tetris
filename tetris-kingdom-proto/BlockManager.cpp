#include "BlockManager.h"
#include <algorithm>

using namespace std;

BlockSpec BlockManager::draw(mt19937& random, const StageRules& rules)
{
    uniform_int_distribution<int> percent(1, 100);
    if (next == Block::shapeCount) {
        for (int i = 0; i < Block::shapeCount; ++i) pieces[i] = {i, false, -1};
        shuffle(pieces.begin(), pieces.end(), random);
        if (percent(random) <= rules.getGiantBagPercent()) {
            const int slot = uniform_int_distribution<int>(0, Block::shapeCount - 1)(random);
            pieces[slot].setGiant(true);
        }
        next = 0;
    }
    BlockSpec piece = pieces[next++];
    if (rules.requiresNormalSize(piece.getShape())) piece.setGiant(false);
    if (percent(random) <= rules.getReinforcePercent()) {
        piece.setReinforcedCell(uniform_int_distribution<int>(0, piece.size() - 1)(random));
    }
    return piece;
}
