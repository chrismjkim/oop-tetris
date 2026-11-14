#include "StageManager.h"

#include <cstdlib>

namespace {

struct Stage
{
    int speed;
    int stick_rate;
    int clear_line;
};

const Stage stage_data[] = {
    {40, 20, 20}, {38, 18, 20}, {35, 18, 20}, {30, 17, 20}, {25, 16, 20},
    {20, 14, 20}, {15, 14, 20}, {10, 13, 20}, {6, 12, 20}, {4, 11, 99999}
};

constexpr int stage_count = sizeof(stage_data) / sizeof(stage_data[0]);

} // namespace

int StageManager::speed() const
{
    return stage_data[level].speed;
}

int StageManager::stick_rate() const
{
    return stage_data[level].stick_rate;
}

int StageManager::clear_line() const
{
    return stage_data[level].clear_line;
}

void StageManager::add_line()
{
    ++lines;
    score += 100 + level * 10 + std::rand() % 10;
}

void StageManager::advance_if_cleared()
{
    if (level < stage_count - 1 && lines >= clear_line()) {
        ++level;
        lines = 0;
    }
}
