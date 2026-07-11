#include "logger.hpp"

#include "asserts.hpp"
#include "platform/platform.hpp"

// TODO: temporary
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

constexpr u64 LOG_MESSAGE_BUFFER_SIZE{32000};

bool InitialiseLogging() {
    // TODO: create log file.
    return true;
}

void ShutdownLogging() {
    // TODO: cleanup logging/write queued entries.
}

void LogOutput(const LogLevel level, const char* message, ...) {
    const char* level_strings[6]{"[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
                                 "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

    int level_index = (int)level;
    if (level_index < (int)LogLevel::Fatal ||
        level_index > (int)LogLevel::Trace) {
        level_index = (int)LogLevel::Error;
    }

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    char raw_message[LOG_MESSAGE_BUFFER_SIZE];
    memset(raw_message, 0, sizeof(raw_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with
    // a "typedef char* va_list" in some cases, and as a result throws a strange
    // error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    int message_length =
        vsnprintf(raw_message, sizeof(raw_message), message, arg_ptr);
    va_end(arg_ptr);

    if (message_length < 0) {
        snprintf(raw_message, sizeof(raw_message), "Log formatting error");
    }

    char formatted_message[LOG_MESSAGE_BUFFER_SIZE];
    int formatted_length =
        snprintf(formatted_message, sizeof(formatted_message), "%s%s\n",
                 level_strings[level_index], raw_message);

    if (formatted_length < 0) {
        printf("[ERROR]: Failed to format log message.\n");
        return;
    }

    if (level <= LogLevel::Error) {
        PlatformConsoleWriteError(formatted_message,
                                  static_cast<u8>(level_index));
    } else {
        PlatformConsoleWrite(formatted_message, static_cast<u8>(level_index));
    }
}

void ReportAssertionFailure(const char* expression, const char* message,
                              const char* file, i32 line) {
    LogOutput(LogLevel::Fatal,
               "Assertion Failure: %s, message: '%s', in file: %s, line: %d",
               expression, message, file, line);
}