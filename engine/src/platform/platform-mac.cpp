#include "platform.hpp"

#if KPLATFORM_APPLE

#include <objc/message.h>
#include <objc/runtime.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "core/input.hpp"

namespace {

using NSUInteger = unsigned long;
using NSInteger = long;

struct NSPoint {
    double x;
    double y;
};

struct NSSize {
    double width;
    double height;
};

struct NSRect {
    NSPoint origin;
    NSSize size;
};

constexpr NSUInteger NSWindowStyleMaskTitled {1UL << 0};
constexpr NSUInteger NSWindowStyleMaskClosable {1UL << 1};
constexpr NSUInteger NSWindowStyleMaskMiniaturizable {1UL << 2};
constexpr NSUInteger NSWindowStyleMaskResizable {1UL << 3};
constexpr NSUInteger NSEventMaskAny {~0UL};
constexpr NSUInteger NSBackingStoreBuffered {2};
constexpr NSInteger NSApplicationActivationPolicyRegular {0};
constexpr NSUInteger NSEventTypeLeftMouseDown {1};
constexpr NSUInteger NSEventTypeLeftMouseUp {2};
constexpr NSUInteger NSEventTypeRightMouseDown {3};
constexpr NSUInteger NSEventTypeRightMouseUp {4};
constexpr NSUInteger NSEventTypeMouseMoved {5};
constexpr NSUInteger NSEventTypeLeftMouseDragged {6};
constexpr NSUInteger NSEventTypeRightMouseDragged {7};
constexpr NSUInteger NSEventTypeKeyDown {10};
constexpr NSUInteger NSEventTypeKeyUp {11};
constexpr NSUInteger NSEventTypeFlagsChanged {12};
constexpr NSUInteger NSEventTypeScrollWheel {22};
constexpr NSUInteger NSEventTypeOtherMouseDown {25};
constexpr NSUInteger NSEventTypeOtherMouseUp {26};
constexpr NSUInteger NSEventTypeOtherMouseDragged {27};
constexpr NSUInteger NSEventModifierFlagCapsLock {1UL << 16};
constexpr NSUInteger NSEventModifierFlagShift {1UL << 17};
constexpr NSUInteger NSEventModifierFlagControl {1UL << 18};
constexpr NSUInteger NSEventModifierFlagOption {1UL << 19};
constexpr NSUInteger NSEventModifierFlagCommand {1UL << 20};

struct InternalState {
    id autoreleasePool;
    id app;
    id window;
    id runLoopMode;
};

template <typename Func>
Func MsgSend() {
    return reinterpret_cast<Func>(objc_msgSend);
}

id ToNSString(const char* text) {
    Class ns_string {reinterpret_cast<Class>(objc_getClass("NSString"))};
    return MsgSend<id (*)(id, SEL, const char*)>()(
        reinterpret_cast<id>(ns_string),
        sel_registerName("stringWithUTF8String:"), text);
}

const char* AnsiColourCode(const u8 colour) {
    switch (colour) {
        case 0:  // Fatal
            return "1;35";
        case 1:  // Error
            return "1;31";
        case 2:  // Warn
            return "1;33";
        case 3:  // Info
            return "1;32";
        case 4:  // Debug
            return "1;36";
        case 5:  // Trace
            return "0;37";
        default:
            return "0";
    }
}

void WriteAnsiColour(FILE* stream, const char* message, const u8 colour) {
    const char* code {AnsiColourCode(colour)};
    std::fprintf(stream, "\x1b[%sm%s\x1b[0m", code, message ? message : "");
}

KeyboardButton TranslateKeyCode(unsigned short keyCode) {
    switch (keyCode) {
        case 0x00:
            return KeyboardButton::A;
        case 0x01:
            return KeyboardButton::S;
        case 0x02:
            return KeyboardButton::D;
        case 0x03:
            return KeyboardButton::F;
        case 0x04:
            return KeyboardButton::H;
        case 0x05:
            return KeyboardButton::G;
        case 0x06:
            return KeyboardButton::Z;
        case 0x07:
            return KeyboardButton::X;
        case 0x08:
            return KeyboardButton::C;
        case 0x09:
            return KeyboardButton::V;
        case 0x0B:
            return KeyboardButton::B;
        case 0x0C:
            return KeyboardButton::Q;
        case 0x0D:
            return KeyboardButton::W;
        case 0x0E:
            return KeyboardButton::E;
        case 0x0F:
            return KeyboardButton::R;
        case 0x10:
            return KeyboardButton::Y;
        case 0x11:
            return KeyboardButton::T;
        case 0x12:
            return KeyboardButton::Numpad1;
        case 0x13:
            return KeyboardButton::Numpad2;
        case 0x14:
            return KeyboardButton::Numpad3;
        case 0x15:
            return KeyboardButton::Numpad4;
        case 0x16:
            return KeyboardButton::Numpad6;
        case 0x17:
            return KeyboardButton::Numpad5;
        case 0x18:
            return KeyboardButton::Plus;
        case 0x19:
            return KeyboardButton::Numpad9;
        case 0x1A:
            return KeyboardButton::Numpad7;
        case 0x1B:
            return KeyboardButton::Minus;
        case 0x1C:
            return KeyboardButton::Numpad8;
        case 0x1D:
            return KeyboardButton::Numpad0;
        case 0x1E:
            return KeyboardButton::Period;
        case 0x1F:
            return KeyboardButton::O;
        case 0x20:
            return KeyboardButton::U;
        case 0x21:
            return KeyboardButton::Semicolon;
        case 0x22:
            return KeyboardButton::I;
        case 0x23:
            return KeyboardButton::P;
        case 0x24:
            return KeyboardButton::Enter;
        case 0x25:
            return KeyboardButton::L;
        case 0x26:
            return KeyboardButton::J;
        case 0x27:
            return KeyboardButton::Grave;
        case 0x28:
            return KeyboardButton::K;
        case 0x29:
            return KeyboardButton::Semicolon;
        case 0x2A:
            return KeyboardButton::Slash;
        case 0x2B:
            return KeyboardButton::Comma;
        case 0x2C:
            return KeyboardButton::Slash;
        case 0x2D:
            return KeyboardButton::N;
        case 0x2E:
            return KeyboardButton::M;
        case 0x2F:
            return KeyboardButton::Period;
        case 0x30:
            return KeyboardButton::Tab;
        case 0x31:
            return KeyboardButton::Space;
        case 0x32:
            return KeyboardButton::Grave;
        case 0x33:
            return KeyboardButton::Backspace;
        case 0x35:
            return KeyboardButton::Escape;
        case 0x41:
            return KeyboardButton::Decimal;
        case 0x43:
            return KeyboardButton::Multiply;
        case 0x45:
            return KeyboardButton::Add;
        case 0x47:
            return KeyboardButton::Numlock;
        case 0x4B:
            return KeyboardButton::Divide;
        case 0x4C:
            return KeyboardButton::Enter;
        case 0x4E:
            return KeyboardButton::Subtract;
        case 0x51:
            return KeyboardButton::NumpadEqual;
        case 0x52:
            return KeyboardButton::Numpad0;
        case 0x53:
            return KeyboardButton::Numpad1;
        case 0x54:
            return KeyboardButton::Numpad2;
        case 0x55:
            return KeyboardButton::Numpad3;
        case 0x56:
            return KeyboardButton::Numpad4;
        case 0x57:
            return KeyboardButton::Numpad5;
        case 0x58:
            return KeyboardButton::Numpad6;
        case 0x59:
            return KeyboardButton::Numpad7;
        case 0x5B:
            return KeyboardButton::Numpad8;
        case 0x5C:
            return KeyboardButton::Numpad9;
        case 0x60:
            return KeyboardButton::F5;
        case 0x61:
            return KeyboardButton::F6;
        case 0x62:
            return KeyboardButton::F7;
        case 0x63:
            return KeyboardButton::F3;
        case 0x64:
            return KeyboardButton::F8;
        case 0x65:
            return KeyboardButton::F9;
        case 0x67:
            return KeyboardButton::F11;
        case 0x69:
            return KeyboardButton::F13;
        case 0x6A:
            return KeyboardButton::F16;
        case 0x6B:
            return KeyboardButton::F14;
        case 0x6D:
            return KeyboardButton::F10;
        case 0x6F:
            return KeyboardButton::F12;
        case 0x71:
            return KeyboardButton::F15;
        case 0x72:
            return KeyboardButton::Insert;
        case 0x73:
            return KeyboardButton::Home;
        case 0x74:
            return KeyboardButton::Prior;
        case 0x75:
            return KeyboardButton::Delete;
        case 0x76:
            return KeyboardButton::F4;
        case 0x77:
            return KeyboardButton::End;
        case 0x78:
            return KeyboardButton::F2;
        case 0x79:
            return KeyboardButton::Next;
        case 0x7A:
            return KeyboardButton::F1;
        case 0x7B:
            return KeyboardButton::Left;
        case 0x7C:
            return KeyboardButton::Right;
        case 0x7D:
            return KeyboardButton::Down;
        case 0x7E:
            return KeyboardButton::Up;
        default:
            return KeyboardButton::_count;
    }
}

KeyboardButton TranslateModifierKey(unsigned short keyCode) {
    switch (keyCode) {
        case 0x36:
            return KeyboardButton::RWin;
        case 0x37:
            return KeyboardButton::LWin;
        case 0x38:
            return KeyboardButton::LShift;
        case 0x39:
            return KeyboardButton::Capital;
        case 0x3A:
            return KeyboardButton::LMenu;
        case 0x3B:
            return KeyboardButton::LControl;
        case 0x3C:
            return KeyboardButton::RShift;
        case 0x3D:
            return KeyboardButton::RMenu;
        case 0x3E:
            return KeyboardButton::RControl;
        default:
            return KeyboardButton::_count;
    }
}

NSUInteger ModifierMaskForKey(KeyboardButton key) {
    switch (key) {
        case KeyboardButton::Capital:
            return NSEventModifierFlagCapsLock;
        case KeyboardButton::LShift:
        case KeyboardButton::RShift:
            return NSEventModifierFlagShift;
        case KeyboardButton::LControl:
        case KeyboardButton::RControl:
            return NSEventModifierFlagControl;
        case KeyboardButton::LMenu:
        case KeyboardButton::RMenu:
            return NSEventModifierFlagOption;
        case KeyboardButton::LWin:
        case KeyboardButton::RWin:
            return NSEventModifierFlagCommand;
        default:
            return 0;
    }
}

MouseButton TranslateMouseButton(NSInteger buttonNumber) {
    switch (buttonNumber) {
        case 0:
            return MouseButton::Left;
        case 1:
            return MouseButton::Right;
        case 2:
            return MouseButton::Middle;
        default:
            return MouseButton::_count;
    }
}

void ProcessMouseMoveEvent(id window, id event) {
    const NSPoint location {MsgSend<NSPoint (*)(id, SEL)>()(
        event, sel_registerName("locationInWindow"))};
    id contentView {
        MsgSend<id (*)(id, SEL)>()(window, sel_registerName("contentView"))};
    const NSRect frame {
        MsgSend<NSRect (*)(id, SEL)>()(contentView, sel_registerName("frame"))};

    const i16 x {static_cast<i16>(location.x)};
    const i16 y {static_cast<i16>(frame.size.height - location.y)};
    InputProcessMouseMove(x, y);
}

void ProcessKeyEvent(id event, bool pressed) {
    const auto keyCode {MsgSend<unsigned short (*)(id, SEL)>()(
        event, sel_registerName("keyCode"))};
    const KeyboardButton key {TranslateKeyCode(keyCode)};
    if (key != KeyboardButton::_count) {
        InputProcessKey(key, pressed);
    }
}

void ProcessModifierEvent(id event) {
    const auto keyCode {MsgSend<unsigned short (*)(id, SEL)>()(
        event, sel_registerName("keyCode"))};
    const KeyboardButton key {TranslateModifierKey(keyCode)};
    if (key == KeyboardButton::_count) {
        return;
    }

    const NSUInteger modifierFlags {MsgSend<NSUInteger (*)(id, SEL)>()(
        event, sel_registerName("modifierFlags"))};
    const bool pressed {(modifierFlags & ModifierMaskForKey(key)) != 0};
    InputProcessKey(key, pressed);
}

void ProcessMouseButtonEvent(id event, bool pressed) {
    const NSInteger buttonNumber {MsgSend<NSInteger (*)(id, SEL)>()(
        event, sel_registerName("buttonNumber"))};
    const MouseButton button {TranslateMouseButton(buttonNumber)};
    if (button != MouseButton::_count) {
        InputProcessButton(button, pressed);
    }
}

void ProcessScrollEvent(id event) {
    const double delta {MsgSend<double (*)(id, SEL)>()(
        event, sel_registerName("scrollingDeltaY"))};
    if (delta > 0.0) {
        InputProcessMouseWheel(1);
    } else if (delta < 0.0) {
        InputProcessMouseWheel(-1);
    }
}

}  // namespace

bool PlatformStartup(PlatformState& platformState, const char* applicationName,
                     i32 x, i32 y, i32 width, i32 height) {
    auto* state {
        static_cast<InternalState*>(std::malloc(sizeof(InternalState)))};
    if (!state) {
        return false;
    }
    std::memset(state, 0, sizeof(InternalState));
    platformState.InternalState = state;

    Class pool_class {
        reinterpret_cast<Class>(objc_getClass("NSAutoreleasePool"))};
    state->autoreleasePool = MsgSend<id (*)(id, SEL)>()(
        reinterpret_cast<id>(pool_class), sel_registerName("new"));

    Class app_class {reinterpret_cast<Class>(objc_getClass("NSApplication"))};
    state->app = MsgSend<id (*)(id, SEL)>()(
        reinterpret_cast<id>(app_class), sel_registerName("sharedApplication"));

    MsgSend<void (*)(id, SEL, NSInteger)>()(
        state->app, sel_registerName("setActivationPolicy:"),
        NSApplicationActivationPolicyRegular);

    Class window_class {reinterpret_cast<Class>(objc_getClass("NSWindow"))};
    id window_alloc {MsgSend<id (*)(id, SEL)>()(
        reinterpret_cast<id>(window_class), sel_registerName("alloc"))};

    NSUInteger style {NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                      NSWindowStyleMaskMiniaturizable |
                      NSWindowStyleMaskResizable};
    NSRect frame {{static_cast<double>(x), static_cast<double>(y)},
                  {static_cast<double>(width), static_cast<double>(height)}};

    state->window =
        MsgSend<id (*)(id, SEL, NSRect, NSUInteger, NSUInteger, bool)>()(
            window_alloc,
            sel_registerName("initWithContentRect:styleMask:backing:defer:"),
            frame, style, NSBackingStoreBuffered, false);
    if (!state->window) {
        return false;
    }

    id title {ToNSString(applicationName ? applicationName : "Kohi")};
    MsgSend<void (*)(id, SEL, id)>()(state->window,
                                     sel_registerName("setTitle:"), title);
    MsgSend<void (*)(id, SEL, bool)>()(
        state->window, sel_registerName("setReleasedWhenClosed:"), false);
    MsgSend<void (*)(id, SEL, id)>()(
        state->window, sel_registerName("makeKeyAndOrderFront:"), nullptr);

    MsgSend<void (*)(id, SEL)>()(state->app,
                                 sel_registerName("finishLaunching"));
    MsgSend<void (*)(id, SEL, bool)>()(
        state->app, sel_registerName("activateIgnoringOtherApps:"), true);

    state->runLoopMode = ToNSString("kCFRunLoopDefaultMode");

    return true;
}

void PlatformShutdown(PlatformState& platformState) {
    if (!platformState.InternalState) {
        return;
    }

    auto* state {static_cast<InternalState*>(platformState.InternalState)};

    if (state->window) {
        MsgSend<void (*)(id, SEL)>()(state->window, sel_registerName("close"));
        state->window = nullptr;
    }

    if (state->autoreleasePool) {
        MsgSend<void (*)(id, SEL)>()(state->autoreleasePool,
                                     sel_registerName("drain"));
        state->autoreleasePool = nullptr;
    }

    std::free(state);
    platformState.InternalState = nullptr;
}

bool PlatformPollMessages(PlatformState& platformState) {
    if (!platformState.InternalState) {
        return false;
    }

    auto* state {static_cast<InternalState*>(platformState.InternalState)};
    if (!state->app || !state->window) {
        return false;
    }

    Class date_class {reinterpret_cast<Class>(objc_getClass("NSDate"))};
    id distant_past {MsgSend<id (*)(id, SEL)>()(
        reinterpret_cast<id>(date_class), sel_registerName("distantPast"))};

    id event {MsgSend<id (*)(id, SEL, NSUInteger, id, id, bool)>()(
        state->app,
        sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        NSEventMaskAny, distant_past, state->runLoopMode, true)};

    while (event) {
        const NSUInteger eventType {MsgSend<NSUInteger (*)(id, SEL)>()(
            event, sel_registerName("type"))};
        switch (eventType) {
            case NSEventTypeKeyDown:
                ProcessKeyEvent(event, true);
                break;
            case NSEventTypeKeyUp:
                ProcessKeyEvent(event, false);
                break;
            case NSEventTypeFlagsChanged:
                ProcessModifierEvent(event);
                break;
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeOtherMouseDragged:
                ProcessMouseMoveEvent(state->window, event);
                break;
            case NSEventTypeScrollWheel:
                ProcessScrollEvent(event);
                break;
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeOtherMouseDown:
                ProcessMouseButtonEvent(event, true);
                break;
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseUp:
                ProcessMouseButtonEvent(event, false);
                break;
            default:
                break;
        }

        MsgSend<void (*)(id, SEL, id)>()(state->app,
                                         sel_registerName("sendEvent:"), event);
        event = MsgSend<id (*)(id, SEL, NSUInteger, id, id, bool)>()(
            state->app,
            sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
            NSEventMaskAny, distant_past, state->runLoopMode, true);
    }

    MsgSend<void (*)(id, SEL)>()(state->app, sel_registerName("updateWindows"));

    return MsgSend<bool (*)(id, SEL)>()(state->window,
                                        sel_registerName("isVisible"));
}

void* PlatformAllocate(u64 size, bool aligned) {
    (void)aligned;
    return std::malloc(static_cast<size_t>(size));
}

void PlatformFree(void* block, bool aligned) {
    (void)aligned;
    std::free(block);
}

void* PlatformZeroMemory(void* block, u64 size) {
    return std::memset(block, 0, static_cast<size_t>(size));
}

void* PlatformCopyMemory(void* destination, const void* source, u64 size) {
    return std::memcpy(destination, source, static_cast<size_t>(size));
}

void* PlatformSetMemory(void* destination, const i32 value, u64 size) {
    return std::memset(destination, value, static_cast<size_t>(size));
}

void PlatformConsoleWrite(const char* message, u8 colour) {
    WriteAnsiColour(stdout, message, colour);
}

void PlatformConsoleWriteError(const char* message, u8 colour) {
    WriteAnsiColour(stderr, message, colour);
}

f64 PlatformGetAbsoluteTime() {
    using clock = std::chrono::steady_clock;

    const auto now {clock::now().time_since_epoch()};

    return std::chrono::duration<f64>(now).count();
}

void PlatformSleep(u64 milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif  // KPLATFORM_APPLE