#include "application.hpp"

#include "game-types.hpp"
#include "logger.hpp"
#include "platform/platform.hpp"

struct ApplicationState {
    Game* gameInstance;
    bool isRunning;
    bool isSuspended;
    PlatformState platform;
    i16 width;
    i16 height;
    f64 lastTime;
};

static bool initialised {false};
static ApplicationState appState {};

bool ApplicationCreate(Game& game) {
    if (initialised) {
        KERROR("ApplicationCreate() called more than once.")
        return false;
    }

    appState.gameInstance = &game;

    // Initialise subsystems.
    InitialiseLogging();

    // TODO: Remove this
    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);
    KDEBUG("A test message: %f", 3.14f);
    KTRACE("A test message: %f", 3.14f);

    appState.isRunning = true;
    appState.isSuspended = false;

    bool platformStartupSuccess {
        PlatformStartup(&appState.platform, game.applicationConfiguration.name,
                        game.applicationConfiguration.startPositionX,
                        game.applicationConfiguration.startPositionY,
                        game.applicationConfiguration.startWidth,
                        game.applicationConfiguration.startHeight)};

    if (!platformStartupSuccess) {
        return false;
    }

    // Initialise the game.
    if (!appState.gameInstance->initialise(*appState.gameInstance)) {
        KFATAL("Game failed to initialise");
        return false;
    }

    appState.gameInstance->onResize(*appState.gameInstance, appState.width,
                                    appState.height);

    initialised = true;

    return true;
}

bool ApplicationRun() {
    while (appState.isRunning) {
        if (!PlatformPollMessages(&appState.platform)) {
            appState.isRunning = false;
        }

        if (!appState.isSuspended) {
            if (!appState.gameInstance->update(*appState.gameInstance, (f32)0)) {
                KFATAL("Game update failed, shutting down.");
                appState.isRunning = false;
                break;
            }

            if (!appState.gameInstance->render(*appState.gameInstance, (f32)0)) {
                KFATAL("Game render failed, shutting down.");
                appState.isRunning = false;
                break;
            }
        }
    }

    appState.isRunning = false;

    PlatformShutdown(&appState.platform);

    return true;
}