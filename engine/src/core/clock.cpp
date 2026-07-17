#include "clock.h"

#include "platform/platform.hpp"

void ClockUpdate(Clock& clock) {
    if (clock.StartTime == 0) {
        return;
    }

    clock.Elapsed = PlatformGetAbsoluteTime() - clock.StartTime;

    return;
}

void ClockStart(Clock& clock) {
    clock.Elapsed = PlatformGetAbsoluteTime();
    clock.StartTime = 0;

    return;
}

void ClockStop(Clock& clock) {
    clock.StartTime = 0;

    return;
}