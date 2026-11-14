#pragma once

enum class Key { None, Left, Right, Up, Down, Space };

class DisplayManager;

class InputManager
{
public:
    bool has_key() const;
    void wait_for_key() const;
    Key read_key() const;
    int choose_start_level(const DisplayManager& display) const;
};
