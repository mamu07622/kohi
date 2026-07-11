#include "platform.hpp"

#if KPLATFORM_APPLE

#include <objc/message.h>
#include <objc/runtime.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

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

}  // namespace

bool PlatformStartup(PlatformState* platformState, const char* applicationName,
                     i32 x, i32 y, i32 width, i32 height) {
    if (!platformState) {
        return false;
    }

    auto* state {
        static_cast<InternalState*>(std::malloc(sizeof(InternalState)))};
    if (!state) {
        return false;
    }
    std::memset(state, 0, sizeof(InternalState));
    platformState->internalState = state;

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

void PlatformShutdown(PlatformState* platformState) {
    if (!platformState || !platformState->internalState) {
        return;
    }

    auto* state {static_cast<InternalState*>(platformState->internalState)};

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
    platformState->internalState = nullptr;
}

bool PlatformPollMessages(PlatformState* platformState) {
    if (!platformState || !platformState->internalState) {
        return false;
    }

    auto* state {static_cast<InternalState*>(platformState->internalState)};
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