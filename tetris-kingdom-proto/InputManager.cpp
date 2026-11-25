#include "InputManager.h"
#include <conio.h>

using namespace std;

InputManager::InputManager()
{
    HANDLE handle = CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle != INVALID_HANDLE_VALUE) localInput.reset(handle);
}

InputManager::~InputManager() = default;

InputManager::LocalKey InputManager::localBinding(const KEY_EVENT_RECORD& key)
{
    const bool enhanced = (key.dwControlKeyState & ENHANCED_KEY) != 0;
    switch (key.wVirtualKeyCode) {
    case VK_ESCAPE: return LocalKey::Escape;
    case 'R': return LocalKey::Restart;
    case 'I': return LocalKey::FirstRotate;
    case 'J': return LocalKey::FirstLeft;
    case 'K': return LocalKey::FirstSoftDrop;
    case 'L': return LocalKey::FirstRight;
    case 'W':
    case VK_LSHIFT: return LocalKey::FirstHold;
    case VK_SHIFT: return key.wVirtualScanCode == 0x2A ? LocalKey::FirstHold : LocalKey::Ignored;
    case 'S':
    case VK_SPACE: return LocalKey::FirstHardDrop;
    case VK_NUMPAD5: return LocalKey::SecondRotate;
    case VK_NUMPAD1: return LocalKey::SecondLeft;
    case VK_NUMPAD2: return LocalKey::SecondSoftDrop;
    case VK_NUMPAD3: return LocalKey::SecondRight;
    // Num Lock이 꺼져도 같은 물리 키를 사용한다. 방향키의 확장 비트로 구분한다.
    case VK_CLEAR: return !enhanced && key.wVirtualScanCode == 0x4C ? LocalKey::SecondRotate : LocalKey::Ignored;
    case VK_END: return !enhanced && key.wVirtualScanCode == 0x4F ? LocalKey::SecondLeft : LocalKey::Ignored;
    case VK_NEXT: return !enhanced && key.wVirtualScanCode == 0x51 ? LocalKey::SecondRight : LocalKey::Ignored;
    case VK_UP: return enhanced ? LocalKey::SecondHold : LocalKey::Ignored;
    case VK_DOWN: return enhanced ? LocalKey::SecondHardDrop :
        (key.wVirtualScanCode == 0x50 ? LocalKey::SecondSoftDrop : LocalKey::Ignored);
    default: return LocalKey::Ignored;
    }
}

bool InputManager::keyAvailable() const
{
    return _kbhit() != 0;
}

int InputManager::readKey() const
{
    return _getch();
}

InputManager::LocalKey InputManager::readLocalKey()
{
    DWORD available = 0;
    if (!GetNumberOfConsoleInputEvents(localInput.get(), &available) || available == 0) return LocalKey::None;

    INPUT_RECORD record{};
    DWORD read = 0;
    if (!ReadConsoleInputW(localInput.get(), &record, 1, &read) || read == 0) return LocalKey::None;
    if (record.EventType == FOCUS_EVENT && !record.Event.FocusEvent.bSetFocus) localPressed.fill(false);
    if (record.EventType != KEY_EVENT) return LocalKey::Ignored;

    const KEY_EVENT_RECORD& key = record.Event.KeyEvent;
    const LocalKey action = localBinding(key);
    if (action == LocalKey::Ignored) return action;
    const auto vk = key.wVirtualKeyCode;
    const bool pressed_before = localPressed[vk];
    localPressed[vk] = key.bKeyDown != FALSE;
    if (!key.bKeyDown) return LocalKey::Ignored;
    const bool one_shot = action == LocalKey::FirstHold || action == LocalKey::FirstHardDrop ||
        action == LocalKey::SecondHold || action == LocalKey::SecondHardDrop || action == LocalKey::Restart;
    return one_shot && pressed_before ? LocalKey::Ignored : action;
}

void InputManager::discardPendingKeys()
{
    localPressed.fill(false);
    while (keyAvailable()) readKey();
}
