#pragma once

#if defined(__has_include)
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

#include "defines.hpp"

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
#define K_HAS_SOURCE_LOCATION 1
#else
#define K_HAS_SOURCE_LOCATION 0
#endif

// Assertions are enabled by default in non-release builds. Define
// KASSERTIONS_ENABLED to 0 or 1 before including this header to override.
#ifndef KASSERTIONS_ENABLED
#if defined(NDEBUG)
#define KASSERTIONS_ENABLED 0
#else
#define KASSERTIONS_ENABLED 1
#endif
#endif

#if KASSERTIONS_ENABLED
#if defined(_MSC_VER)
#include <intrin.h>
#define K_DEBUG_BREAK() __debugbreak()
#elif defined(__has_builtin)
#if __has_builtin(__builtin_debugtrap)
#define K_DEBUG_BREAK() __builtin_debugtrap()
#else
#define K_DEBUG_BREAK() __builtin_trap()
#endif
#else
#define K_DEBUG_BREAK() __builtin_trap()
#endif

KAPI void ReportAssertionFailure(const char* expression, const char* message,
                                 const char* file, i32 line);

namespace kassert_detail {

struct SourceLocation {
    const char* file;
    i32 line;

#if K_HAS_SOURCE_LOCATION
    static SourceLocation current(
        const std::source_location location = std::source_location::current()) {
        return {location.file_name(), static_cast<i32>(location.line())};
    }
#else
    static SourceLocation current(const char* file_name, i32 line_number) {
        return {file_name, line_number};
    }
#endif
};

inline void TriggerDebugBreak() { K_DEBUG_BREAK(); }

inline void HandleAssertion(const bool expressionResult, const char* expression,
                            const char* message,
                            const SourceLocation location) {
    if (expressionResult) {
        return;
    }

    ReportAssertionFailure(expression, message ? message : "", location.file,
                           location.line);
    TriggerDebugBreak();
}

}  // namespace kassert_detail

#if K_HAS_SOURCE_LOCATION
#define K_SOURCE_LOCATION_CURRENT() kassert_detail::SourceLocation::current()
#else
#define K_SOURCE_LOCATION_CURRENT() \
    kassert_detail::SourceLocation::current(__FILE__, __LINE__)
#endif

#define KASSERT(expr)                                                 \
    do {                                                              \
        kassert_detail::HandleAssertion((expr), #expr, "",            \
                                        K_SOURCE_LOCATION_CURRENT()); \
    } while (0)

#define KASSERT_MSG(expr, message)                                    \
    do {                                                              \
        kassert_detail::HandleAssertion((expr), #expr, (message),     \
                                        K_SOURCE_LOCATION_CURRENT()); \
    } while (0)

#if defined(_DEBUG)
#define KASSERT_DEBUG(expr)                                           \
    do {                                                              \
        kassert_detail::HandleAssertion((expr), #expr, "",            \
                                        K_SOURCE_LOCATION_CURRENT()); \
    } while (0)
#else
#define KASSERT_DEBUG(expr)  // Does nothing at all
#endif

#else
#define KASSERT(expr)               // Does nothing at all
#define KASSERT_MSG(expr, message)  // Does nothing at all
#define KASSERT_DEBUG(expr)         // Does nothing at all
#endif

#undef K_HAS_SOURCE_LOCATION