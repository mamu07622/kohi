#include "application.hpp"

#include "containers/darray.h"
#include "event.h"
#include "game-types.hpp"
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

bool ApplicationCreate(Game& game) {
    if (sInitialised) {
        KERROR("ApplicationCreate() called more than once.")
        return false;
    }

    sApplicationState.GameInstance = &game;

    // Initialise subsystems.
    InitialiseLogging();

    // TODO: Remove this
    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);
    KDEBUG("A test message: %f", 3.14f);
    KTRACE("A test message: %f", 3.14f);

    sApplicationState.IsApplicationRunning = true;
    sApplicationState.IsApplicationSuspended = false;

    if (!EventInitialise()) {
        KERROR(
            "Event system failed initialization. Application cannot continue.");
        return false;
    }

    ApplicationConfiguration& appConfig {game.ApplicationConfiguration};

    bool platformStartupSuccess {PlatformStartup(
        sApplicationState.Platform, appConfig.name, appConfig.StartPositionX,
        appConfig.StartPositionY, appConfig.StartWidth, appConfig.StartHeight)};

    if (!platformStartupSuccess) {
        return false;
    }

    // Initialise the game.
    if (!sApplicationState.GameInstance->Initialise(
            *sApplicationState.GameInstance)) {
        KFATAL("Game failed to initialise");
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

    KINFO("DArray test: length=%llu, value=%d", DArrayLength(darrayTest),
          poppedValue);

    KINFO(GetMemoryUsageString());

    DArrayDestroy(darrayTest);

    while (sApplicationState.IsApplicationRunning) {
        if (!PlatformPollMessages(sApplicationState.Platform)) {
            sApplicationState.IsApplicationRunning = false;
        }

        if (!sApplicationState.IsApplicationSuspended) {
            if (!sApplicationState.GameInstance->Update(
                    *sApplicationState.GameInstance, (f32)0)) {
                KFATAL("Game update failed, shutting down.");
                sApplicationState.IsApplicationRunning = false;
                break;
            }

            if (!sApplicationState.GameInstance->Render(
                    *sApplicationState.GameInstance, (f32)0)) {
                KFATAL("Game render failed, shutting down.");
                sApplicationState.IsApplicationRunning = false;
                break;
            }
        }
    }

    sApplicationState.IsApplicationRunning = false;

    ShutdownEvent();

    PlatformShutdown(sApplicationState.Platform);

    return true;
}