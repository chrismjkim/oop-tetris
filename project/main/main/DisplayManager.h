#pragma once

class Block;
class BlockManager;
class StageManager;

class DisplayManager
{
public:
    void initialize_console() const;
    void show_logo() const;
    void animate_logo() const;
    void show_controls() const;
    void show_level_prompt() const;
    void show_cur_block(const Block& piece) const;
    void erase_cur_block(const Block& piece) const;
    void show_total_block(const BlockManager& board, int level) const;
    void show_next_block(int shape, int level) const;
    void show_gamestat(const StageManager& stage) const;
    void animate_clear_line(int row) const;
    void show_gameover() const;
    void clear() const;
    void wait(int milliseconds) const;
    void park_cursor() const;
};
