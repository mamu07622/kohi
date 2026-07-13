#include "logger.hpp"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include "asserts.hpp"
#include "platform/platform.hpp"

namespace {

std::string VFormatMessage(const char* format, va_list args) {
    if (!format) {
        return "Log formatting error";
    }

    va_list size_args;
    va_copy(size_args, args);
    const int required = std::vsnprintf(nullptr, 0, format, size_args);
    va_end(size_args);

    if (required < 0) {
        return "Log formatting error";
    }

    std::vector<char> buffer(static_cast<size_t>(required) + 1);
    const int written =
        std::vsnprintf(buffer.data(), buffer.size(), format, args);
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
}

void LogOutput(const LogLevel level, const char* message, ...) {
    constexpr std::array<const char*, 6> level_strings {
        "[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
        "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

    int level_index = (int)level;
    if (level_index < (int)LogLevel::Fatal ||
        level_index > (int)LogLevel::Trace) {
        level_index = (int)LogLevel::Error;
    }

    va_list arg_ptr;
    va_start(arg_ptr, message);
    std::string raw_message {VFormatMessage(message, arg_ptr)};
    va_end(arg_ptr);

    std::string formatted_message {};
    formatted_message.reserve(
        std::char_traits<char>::length(level_strings[level_index]) +
        raw_message.size() + 1);

    formatted_message.append(level_strings[level_index]);
    formatted_message.append(raw_message);
    
    formatted_message.push_back('\n');

    if (level <= LogLevel::Error) {
        PlatformConsoleWriteError(formatted_message.c_str(),
                                  static_cast<u8>(level_index));
    } else {
        PlatformConsoleWrite(formatted_message.c_str(),
                             static_cast<u8>(level_index));
    }
}

void ReportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 line) {
    LogOutput(LogLevel::Fatal,
              "Assertion Failure: %s, message: '%s', in file: %s, line: %d",
              expression, message, file, line);
}