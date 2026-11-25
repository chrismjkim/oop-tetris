#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <array>
#include <memory>

using namespace std;

class InputManager {
public:
    enum class LocalKey {
        None, Ignored, Escape, Restart,
        FirstRotate, FirstLeft, FirstSoftDrop, FirstRight, FirstHold, FirstHardDrop,
        SecondRotate, SecondLeft, SecondSoftDrop, SecondRight, SecondHold, SecondHardDrop
    };

    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    bool keyAvailable() const;
    int readKey() const;
    LocalKey readLocalKey();
    void discardPendingKeys();

private:
    static LocalKey localBinding(const KEY_EVENT_RECORD& key);
    unique_ptr<void, decltype(&CloseHandle)> localInput{nullptr, &CloseHandle};
    array<bool, 256> localPressed{};
};
