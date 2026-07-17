#include "application.hpp"

#include "containers/darray.h"
#include "core/clock.h"
#include "event.h"
#include "game-types.hpp"
#include "input.hpp"
#include "kmemory.hpp"
#include "logger.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer-frontend.hpp"

struct ApplicationState {
    Game* GameInstance;
    bool IsApplicationRunning;
    bool IsApplicationSuspended;
    PlatformState Platform;
    i16 WindowWidth;
    i16 WindowHeight;
    Clock Clock;
    f64 PreviousFrameTime;
};

static bool sInitialised {false};
static ApplicationState sApplicationState;

// Event handlers
bool ApplicationOnEvent(SystemEventCode code, void* sender, void* listener,
                        EventContext context);
bool ApplicationOnKey(SystemEventCode code, void* sender, void* listener,
                      EventContext context);

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

    EventRegister(SystemEventCode::ApplicationQuit, nullptr,
                  ApplicationOnEvent);
    EventRegister(SystemEventCode::KeyPressed, nullptr, ApplicationOnKey);
    EventRegister(SystemEventCode::KeyReleased, nullptr, ApplicationOnKey);

    bool platformStartupSuccess {PlatformStartup(
        sApplicationState.Platform, appConfig.name, appConfig.StartPositionX,
        appConfig.StartPositionY, appConfig.StartWidth, appConfig.StartHeight)};

    if (!platformStartupSuccess) {
        return false;
    }

    // Renderer startup
    if (!RendererInitialise(appConfig.name, sApplicationState.Platform)) {
        KFATAL("Failed to initialize renderer. Aborting application.");
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
    ClockStart(sApplicationState.Clock);
    ClockUpdate(sApplicationState.Clock);
    sApplicationState.PreviousFrameTime = sApplicationState.Clock.Elapsed;
    // f64 runningTime {};
    // u8 frameCount {};
    f64 targetFrameSeconds {1.0f / 60};

    KINFO(GetMemoryUsageString());

    while (sApplicationState.IsApplicationRunning) {
        if (!PlatformPollMessages(sApplicationState.Platform)) {
            sApplicationState.IsApplicationRunning = false;
        }

        if (!sApplicationState.IsApplicationSuspended) {
            // Update clock and get delta time.
            ClockUpdate(sApplicationState.Clock);
            f64 currentTime {sApplicationState.Clock.Elapsed};
            f64 delta {currentTime - sApplicationState.PreviousFrameTime};
            f64 frameStartTime {PlatformGetAbsoluteTime()};

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

            // TODO: refactor packet creation
            RenderPacket packet {.deltaTime = static_cast<f32>(delta)};
            RendererDrawFrame(packet);

            // Figure out how long the frame took and, if below
            f64 frameEndTime {PlatformGetAbsoluteTime()};
            f64 frameElapsedTime {frameEndTime - frameStartTime};
            // runningTime += frameElapsedTime;
            f64 remainingSeconds {targetFrameSeconds - frameElapsedTime};

            if (remainingSeconds > 0) {
                u64 remainingMs {static_cast<u64>(remainingSeconds * 1000)};

                // If there is time left, give it back to the OS.
                bool limitFrames {false};
                if (remainingMs > 0 && limitFrames) {
                    PlatformSleep(remainingMs - 1);
                }

                // frameCount++;
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

    EventUnregister(SystemEventCode::ApplicationQuit, nullptr,
                    ApplicationOnEvent);
    EventUnregister(SystemEventCode::KeyPressed, nullptr, ApplicationOnKey);
    EventUnregister(SystemEventCode::MouseButtonReleased, nullptr,
                    ApplicationOnKey);

    ShutdownEvent();
    ShutdownInput();

    RendererShutdown();

    PlatformShutdown(sApplicationState.Platform);

    return true;
}

bool ApplicationOnEvent(SystemEventCode code, void* sender, void* listener,
                        EventContext context) {
    switch (code) {
        case SystemEventCode::ApplicationQuit: {
            KINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            sApplicationState.IsApplicationRunning = false;
            return true;
        }
        default: {
            return false;
        }
    }

    return false;
}

bool ApplicationOnKey(SystemEventCode code, void* sender, void* listener,
                      EventContext context) {
    if (code == SystemEventCode::KeyPressed) {
        auto keyCode {static_cast<KeyboardButton>(context.data.u16[0])};

        if (keyCode == KeyboardButton::Escape) {
            // NOTE: Technically firing an event to itself, but there may be
            // other listeners.
            EventContext data {};
            EventFire(SystemEventCode::KeyPressed, 0, data);

            // Block anything else from processing this.
            return true;
        } else if (keyCode == KeyboardButton::A) {
            // Example on checking for a key
            KDEBUG("Explicit - A key pressed!");
        } else {
            KDEBUG("'%c' key pressed in window.", keyCode);
        }
    } else if (code == SystemEventCode::KeyReleased) {
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