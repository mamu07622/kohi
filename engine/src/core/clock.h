#pragma once

#include "defines.hpp"

struct Clock {
    f64 StartTime;
    f64 Elapsed;
};

// Updates the provided clock. SHould be called just before checking the elapsed
// time. Has no effect on non-started clocks.
void ClockUpdate(Clock& clock);

// Starts the provided clock. Resets elapsed time.
void ClockStart(Clock& clock);

// Stops the provided clock. Does not reset elapsed time.
void ClockStop(Clock& clock);