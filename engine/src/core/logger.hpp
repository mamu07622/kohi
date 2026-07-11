#pragma once

#include "defines.hpp"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

enum class LogLevel : u8 {
    Fatal = 0,
    Error = 1,
    Warn = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

bool InitialiseLogging();
void ShutdownLogging();

KAPI void LogOutput(LogLevel level, const char* message, ...);

// Logs a fatal-level message.
#define KFATAL(message, ...) LogOutput(LogLevel::Fatal, message, ##__VA_ARGS__);

#ifndef KERROR
// Logs an error-level message.
#define KERROR(message, ...) LogOutput(LogLevel::Error, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
#define KWARN(message, ...) LogOutput(LogLevel::Warn, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define KWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
#define KINFO(message, ...) LogOutput(LogLevel::Info, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define KINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
#define KDEBUG(message, ...) LogOutput(LogLevel::Debug, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define KDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
#define KTRACE(message, ...) LogOutput(LogLevel::Trace, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define KTRACE(message, ...)
#endif