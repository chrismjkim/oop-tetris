#include "InputManager.h"

#include "DisplayManager.h"

#include <conio.h>
#include <iostream>
#include <limits>

bool InputManager::has_key() const
{
    return _kbhit() != 0;
}

void InputManager::wait_for_key() const
{
    const int key = _getch();
    if (key == 0 || key == 224) {
        _getch();
    }
}

Key InputManager::read_key() const
{
    if (!has_key()) {
        return Key::None;
    }

    const int key = _getch();
    if (key == ' ') {
        return Key::Space;
    }
    if (key != 0 && key != 224) {
        return Key::None;
    }
    switch (_getch()) {
    case 0x4b: return Key::Left;
    case 0x4d: return Key::Right;
    case 0x48: return Key::Up;
    case 0x50: return Key::Down;
    default: return Key::None;
    }
}

int InputManager::choose_start_level(const DisplayManager& display) const
{
    while (true) {
        display.show_level_prompt();
        int level = 0;
        if (std::cin >> level) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (level >= 1 && level <= 8) {
                return level - 1;
            }
        } else {
            if (std::cin.eof() || std::cin.bad()) {
                return -1;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}
