#include "logger.hpp"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "asserts.hpp"
#include "platform/platform.hpp"

namespace {

std::string FormatLogMessage(const char* format, va_list args) {
    if (!format) {
        return "Log formatting error";
    }

    va_list sizeArgs;
    va_copy(sizeArgs, args);
    const int required = std::vsnprintf(nullptr, 0, format, sizeArgs);
    va_end(sizeArgs);

    if (required < 0) {
        return "Log formatting error";
    }

    std::vector<char> buffer(static_cast<size_t>(required) + 1);
    const int written {
        std::vsnprintf(buffer.data(), buffer.size(), format, args)};
    if (written < 0) {
        return "Log formatting error";
    }

    return std::string(buffer.data(), static_cast<size_t>(written));
}

}  // namespace

bool InitialiseLogging() {
    // TODO: create log file.
    return true;
}

void ShutdownLogging() {
    // TODO: cleanup logging/write queued entries.
    return;
}

void LogOutput(const LogLevel level, const char* message, ...) {
    constexpr std::array<const char*, 6> levelStrings {
        "[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
        "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

    auto levelIndex {std::to_underlying(level)};
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levelStrings.size())) {
        levelIndex = std::to_underlying(LogLevel::Error);
    }

    va_list argPtr;
    va_start(argPtr, message);
    std::string rawMessage {FormatLogMessage(message, argPtr)};
    va_end(argPtr);

    std::string formattedMessage {};
    formattedMessage.reserve(
        std::char_traits<char>::length(levelStrings[levelIndex]) +
        rawMessage.size() + 1);

    formattedMessage.append(levelStrings[levelIndex]);
    formattedMessage.append(rawMessage);

    formattedMessage.push_back('\n');

    if (level <= LogLevel::Error) {
        PlatformConsoleWriteError(formattedMessage.c_str(),
                                  static_cast<u8>(levelIndex));
    } else {
        PlatformConsoleWrite(formattedMessage.c_str(),
                             static_cast<u8>(levelIndex));
    }
}

void ReportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 line) {
    LogOutput(LogLevel::Fatal,
              "Assertion Failure: %s, message: '%s', in file: %s, line: %d",
              expression, message, file, line);
}