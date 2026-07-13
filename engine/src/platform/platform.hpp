#pragma once

#include "defines.hpp"

struct PlatformState {
    void* InternalState;
};

bool PlatformStartup(PlatformState& platformState, const char* applicationName,
                     i32 x, i32 y, i32 width, i32 height);

void PlatformShutdown(PlatformState& platformState);

bool PlatformPollMessages(PlatformState& platformState);

void* PlatformAllocate(u64 size, bool aligned = false);
void PlatformFree(void* block, bool aligned = false);
void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* destination, const void* source, u64 size);
void* PlatformSetMemory(void* destination, const i32 value, u64 size);

void PlatformConsoleWrite(const char* message, u8 colour);
void PlatformConsoleWriteError(const char* message, u8 colour);

f64 PlatformGetAbsoluteTime();

// Sleep on thread for the provided ms. This blocks the main thread.
// Should only be used for giving time back to the OS for unusued update power.
// Therefore it is not exported.
void PlatformSleep(u64 milliseconds);