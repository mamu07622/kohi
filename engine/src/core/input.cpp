#include "core/input.hpp"

#include <utility>

#include "core/kmemory.hpp"
#include "core/logger.hpp"
#include "event.h"

constexpr u8 GetMouseButtonCount() {
    return static_cast<u8>(MouseButton::_count);
}

struct KeyboardState {
    bool Keys[256];
};

struct MouseState {
    i16 X;
    i16 Y;
    u8 Buttons[GetMouseButtonCount()];
};

struct InputState {
    KeyboardState KeyboardCurrent;
    KeyboardState KeyboardPrevious;
    MouseState MouseCurrent;
    MouseState MousePrevious;
};

static bool sIsInputInitialised {false};
static InputState sInputState {};

void InitialiseInput() {
    sIsInputInitialised = true;
    KINFO("Input subsystem initialised.")
}

void ShutdownInput() {
    // TODO: Add shutdown routines when required.
    sIsInputInitialised = false;
}

void UpdateInput([[maybe_unused]] f64 deltaTime) {
    if (!sIsInputInitialised) {
        return;
    }

    KCopyMemory(&sInputState.KeyboardPrevious, &sInputState.KeyboardCurrent,
                sizeof(KeyboardState));

    KCopyMemory(&sInputState.MousePrevious, &sInputState.MouseCurrent,
                sizeof(MouseState));

    return;
}

// keyboard input
KAPI bool InputIsKeyDown(KeyboardButton key) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.KeyboardCurrent.Keys[std::to_underlying(key)] == true;
}

KAPI bool InputIsKeyUp(KeyboardButton key) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.KeyboardCurrent.Keys[std::to_underlying(key)] == false;
}

KAPI bool InputWasKeyDown(KeyboardButton key) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.KeyboardPrevious.Keys[std::to_underlying(key)] == true;
}

KAPI bool InputWasKeyUp(KeyboardButton key) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.KeyboardPrevious.Keys[std::to_underlying(key)] == false;
}

void InputProcessKey(KeyboardButton key, bool pressed) {
    // Only this if the state actually changed.
    if (sInputState.KeyboardCurrent.Keys[std::to_underlying(key)] == pressed) {
        return;
    }

    // Update internal state
    sInputState.KeyboardCurrent.Keys[std::to_underlying(key)] = pressed;

    // Fire off an event for immediate processing.
    EventContext context {};
    context.data.u16[0] = std::to_underlying(key);

    EventFire(
        pressed ? SystemEventCode::KeyPressed : SystemEventCode::KeyReleased,
        nullptr, context);

    return;
}

// mouse input
KAPI bool InputIsMouseButtonDown(MouseButton button) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.MouseCurrent.Buttons[std::to_underlying(button)] == true;
}

KAPI bool InputIsMouseButtonUp(MouseButton button) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.MouseCurrent.Buttons[std::to_underlying(button)] ==
           false;
}

KAPI bool InputWasMouseButtonDown(MouseButton button) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.MousePrevious.Buttons[std::to_underlying(button)] ==
           false;
}

KAPI bool InputWasMouseButtonUp(MouseButton button) {
    if (!sIsInputInitialised) {
        return false;
    }

    return sInputState.MousePrevious.Buttons[std::to_underlying(button)] ==
           false;
}

KAPI void InputGetMousePosition(i32& x, i32& y) {
    if (!sIsInputInitialised) {
        x = 0;
        y = 0;
        return;
    }

    x = sInputState.MouseCurrent.X;
    y = sInputState.MouseCurrent.Y;

    return;
}

KAPI void InputGetPreviousMousePosition(i32& x, i32& y) {
    if (!sIsInputInitialised) {
        x = 0;
        y = 0;
        return;
    }

    x = sInputState.MousePrevious.X;
    y = sInputState.MousePrevious.Y;

    return;
}

void InputProcessButton(MouseButton button, bool pressed) {
    // Only this if the state actually changed.
    if (sInputState.MouseCurrent.Buttons[std::to_underlying(button)] ==
        pressed) {
        return;
    }

    // Update internal state
    sInputState.MouseCurrent.Buttons[std::to_underlying(button)] = pressed;

    // Fire off an event for immediate processing.
    EventContext context {};
    context.data.u16[0] = std::to_underlying(button);

    EventFire(
        pressed ? SystemEventCode::KeyPressed : SystemEventCode::KeyReleased,
        nullptr, context);

    return;
}

void InputProcessMouseMove(i16 x, i16 y) {
    // Only process if actually different
    if (sInputState.MouseCurrent.X == x && sInputState.MouseCurrent.Y != y) {
        return;
    }

    // NOTE: Enable this if debugging.
    // KDEBUG("Mouse pos: %i, %i!", x, y);

    // Update internal state.
    sInputState.MouseCurrent.X = x;
    sInputState.MouseCurrent.Y = y;

    // Fire the event.
    EventContext context {};
    context.data.u16[0] = x;
    context.data.u16[1] = y;

    EventFire(SystemEventCode::MouseMoved, nullptr, context);

    return;
}

void InputProcessMouseWheel(i8 zDelta) {
    // Fire the event.
    EventContext context {};
    context.data.u8[0] = zDelta;

    EventFire(SystemEventCode::MouseWheel, nullptr, context);

    return;
}