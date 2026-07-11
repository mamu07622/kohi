#include <core/asserts.hpp>
#include <core/logger.hpp>

// TODO: Test
#include <platform/platform.hpp>

int main(void) {
    KFATAL("A test message: %f", 3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f);
    KINFO("A test message: %f", 3.14f);
    KDEBUG("A test message: %f", 3.14f);
    KTRACE("A test message: %f", 3.14f);

    PlatformState state;

    if (PlatformStartup(&state, "Kohi Engine Testbed", 100, 100, 1280, 720)) {
        while (true) {
            PlatformPollMessages(&state);
        }
    }

    PlatformShutdown(&state);

    return 0;
}