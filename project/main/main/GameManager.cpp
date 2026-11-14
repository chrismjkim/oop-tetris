#include "GameManager.h"

#include <cstdlib>
#include <ctime>

void GameManager::run()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    dm.initialize_console();
    dm.show_logo();
    for (int frame = 0; !im.has_key(); frame = (frame + 1) % 40) {
        if (frame == 0) {
            dm.animate_logo();
        }
        dm.wait(30);
    }
    im.wait_for_key();
    dm.clear();

    while (true) {
        bm.reset();
        sm.lines = 0;
        sm.level = 0;
        dm.show_controls();
        const int start_level = im.choose_start_level(dm);
        if (start_level < 0) {
            return;
        }

        sm.level = start_level;
        dm.clear();
        dm.show_total_block(bm, sm.level);
        current = Block{};
        current.shape = bm.make_new_block(sm.stick_rate());
        next_shape = bm.make_new_block(sm.stick_rate());
        dm.show_next_block(next_shape, sm.level);
        dm.show_gamestat(sm);

        for (unsigned frame = 1; ; ++frame) {
            int result = 0;
            Block candidate = current;
            switch (im.read_key()) {
            case Key::Up:
                candidate.angle = (candidate.angle + 1) % Block::size;
                try_move(candidate);
                break;
            case Key::Left:
                --candidate.x;
                try_move(candidate);
                break;
            case Key::Right:
                ++candidate.x;
                try_move(candidate);
                break;
            case Key::Down:
                result = move_block();
                break;
            case Key::Space:
                do {
                    result = move_block();
                } while (result == 0);
                break;
            case Key::None:
                break;
            }

            if (result == 0 && frame % static_cast<unsigned>(sm.speed()) == 0) {
                result = move_block();
            }
            if (result == 1) {
                dm.show_gameover();
                im.wait_for_key();
                dm.clear();
                break;
            }

            const int previous_level = sm.level;
            sm.advance_if_cleared();
            if (sm.level != previous_level) {
                dm.show_gamestat(sm);
            }

            dm.park_cursor();
            dm.wait(15);
        }
    }
}

bool GameManager::try_move(const Block& candidate)
{
    if (bm.strike_check(candidate)) {
        return false;
    }
    dm.erase_cur_block(current);
    current = candidate;
    dm.show_cur_block(current);
    return true;
}

int GameManager::move_block()
{
    Block candidate = current;
    ++candidate.y;

    // 원본 반환값 0은 낙하, 1은 게임 오버, 2는 고정 후 다음 블록 등장이다.
    if (try_move(candidate)) {
        return 0;
    }
    if (!bm.merge_block(current)) {
        return 1;
    }

    for (int row = 0; row < BlockManager::rows - 1; ++row) {
        if (bm.check_full_line(row)) {
            dm.show_total_block(bm, sm.level);
            dm.animate_clear_line(row);
            bm.clear_line(row);
            sm.add_line();
            dm.show_gamestat(sm);
        }
    }
    dm.show_total_block(bm, sm.level);

    current = Block{};
    current.shape = next_shape;
    next_shape = bm.make_new_block(sm.stick_rate());
    dm.show_next_block(next_shape, sm.level);
    dm.show_cur_block(current);
    return 2;
}
