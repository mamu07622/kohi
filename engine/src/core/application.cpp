#include "application.hpp"

#include "containers/darray.h"
#include "event.h"
#include "game-types.hpp"
#include "input.hpp"
#include "kmemory.hpp"
#include "logger.hpp"
#include "platform/platform.hpp"

struct ApplicationState {
    Game* GameInstance;
    bool IsApplicationRunning;
    bool IsApplicationSuspended;
    PlatformState Platform;
    i16 WindowWidth;
    i16 WindowHeight;
    f64 PreviousFrameTime;
};

static bool sInitialised {false};
static ApplicationState sApplicationState;

// Event handlers
bool ApplicationOnEvent(u16 code, void* sender, void* listener,
                        EventContext context);
bool ApplicationOnKey(u16 code, void* sender, void* listener,
                      EventContext context);

namespace {

constexpr u16 EventSmokeCode {1000};

bool OnEventSmoke(u16 code, void* sender, void* listenerInst,
                  EventContext context) {
    (void)sender;

    if (!listenerInst) {
        return false;
    }

    auto* callbackHandled {static_cast<bool*>(listenerInst)};
    *callbackHandled = (code == EventSmokeCode && context.data.i32[0] == 123);

    return *callbackHandled;
}

}  // namespace

bool ApplicationCreate(Game& game) {
    if (sInitialised) {
        KERROR(
            "ApplicationCreate() "
            "called more than once.")
        return false;
    }

    sApplicationState.GameInstance = &game;

    // Initialise subsystems.
    InitialiseLogging();
    InitialiseInput();

    // TODO: Remove this
    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);

    sApplicationState.IsApplicationRunning = true;
    sApplicationState.IsApplicationSuspended = false;

    if (!InitialiseEvent()) {
        KERROR(
            "Event system failed "
            "initialization. "
            "Application cannot "
            "continue.");
        return false;
    }

    ApplicationConfiguration& appConfig {game.ApplicationConfiguration};

    EventRegister(EVENT_CODE_APPLICATION_QUIT, nullptr, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, nullptr, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, nullptr, ApplicationOnKey);

    bool platformStartupSuccess {PlatformStartup(
        sApplicationState.Platform, appConfig.name, appConfig.StartPositionX,
        appConfig.StartPositionY, appConfig.StartWidth, appConfig.StartHeight)};

    if (!platformStartupSuccess) {
        return false;
    }

    // Initialise the game.
    if (!sApplicationState.GameInstance->Initialise(
            *sApplicationState.GameInstance)) {
        KFATAL(
            "Game failed to "
            "initialise");
        return false;
    }

    sApplicationState.GameInstance->OnResize(*sApplicationState.GameInstance,
                                             sApplicationState.WindowWidth,
                                             sApplicationState.WindowHeight);

    sInitialised = true;

    return true;
}

bool ApplicationRun() {
    // DArray Test
    auto* darrayTest {DArrayCreate<i32>()};
    DArrayPush(darrayTest, 42);

    i32 poppedValue {};
    DArrayPop(darrayTest, &poppedValue);

    KINFO(
        "DArray test: length=%llu, "
        "value=%d",
        DArrayLength(darrayTest), poppedValue);

    // EventSystem smoke test.
    bool callbackHandled {false};
    EventContext eventContext {};
    eventContext.data.i32[0] = 123;

    const bool registerSuccess {
        EventRegister(EventSmokeCode, &callbackHandled, OnEventSmoke)};
    bool fireHandled {false};
    bool unregisterSuccess {false};

    if (registerSuccess) {
        fireHandled = EventFire(EventSmokeCode, nullptr, eventContext);
        unregisterSuccess =
            EventUnregister(EventSmokeCode, &callbackHandled, OnEventSmoke);
    }

    KINFO(
        "EventSystem test: "
        "register=%d, fired=%d, "
        "callback=%d, unregister=%d",
        registerSuccess, fireHandled, callbackHandled, unregisterSuccess);

    KINFO(GetMemoryUsageString());

    DArrayDestroy(darrayTest);

    while (sApplicationState.IsApplicationRunning) {
        if (!PlatformPollMessages(sApplicationState.Platform)) {
            sApplicationState.IsApplicationRunning = false;
        }

        if (!sApplicationState.IsApplicationSuspended) {
            if (!sApplicationState.GameInstance->Update(
                    *sApplicationState.GameInstance, (f32)0)) {
                KFATAL(
                    "Game update "
                    "failed, shutting "
                    "down.");
                sApplicationState.IsApplicationRunning = false;
                break;
            }

            if (!sApplicationState.GameInstance->Render(
                    *sApplicationState.GameInstance, (f32)0)) {
                KFATAL(
                    "Game render "
                    "failed, shutting "
                    "down.");
                sApplicationState.IsApplicationRunning = false;
                break;
            }

            // NOTE: Input update/state
            // copying should always be
            // handled after any input
            // should be recorded; I.E.
            // before this line. As a
            // safety, input is the
            // last thing to be updated
            // before this frame ends.
            UpdateInput(0);
        }
    }

    sApplicationState.IsApplicationRunning = false;

    EventUnregister(EVENT_CODE_APPLICATION_QUIT, nullptr, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, nullptr, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, nullptr, ApplicationOnKey);

    ShutdownEvent();
    ShutdownInput();

    PlatformShutdown(sApplicationState.Platform);

    return true;
}

bool ApplicationOnEvent(u16 code, void* sender, void* listener,
                        EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            KINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            sApplicationState.IsApplicationRunning = false;
            return true;
        }
    }

    return false;
}

bool ApplicationOnKey(u16 code, void* sender, void* listener,
                      EventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        auto keyCode {static_cast<KeyboardButton>(context.data.u16[0])};

        if (keyCode == KeyboardButton::Escape) {
            // NOTE: Technically firing an event to itself, but there may be
            // other listeners.
            EventContext data {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this.
            return true;
        } else if (keyCode == KeyboardButton::A) {
            // Example on checking for a key
            KDEBUG("Explicit - A key pressed!");
        } else {
            KDEBUG("'%c' key pressed in window.", keyCode);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        auto keyCode {static_cast<KeyboardButton>(context.data.u16[0])};

        if (keyCode == KeyboardButton::B) {
            // Example on checking for a key
            KDEBUG("Explicit - B key released!");
        } else {
            KDEBUG("'%c' key released in window.", keyCode);
        }
    }

    return false;
}