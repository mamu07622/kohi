#include "platform.hpp"

// Windows Platform Layer
#if KPLATFORM_WINDOWS

#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "core/logger.h"

struct InternalState {
    HINSTANCE h_instance;
    HWND hwnd;
};

// Clock
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK Win32ProcessMassage(HWND hwnd, u32 msg, WPARAM wparam,
                                     LPARAM lparam);

bool PlatformStartup(PlatformState& platformState, const char* applicationName,
                     i32 x, i32 y, i32 width, i32 height) {
    platformState->internalState = malloc(sizeof(InternalState));

    auto* state {static_cast<InternalState*>(platformState->internalState)};

    state->h_instance {GetModuleHandleA(0)};

    // Setup and register window class.
    WNDCLASSA wc {
        .style = CS_DBLCLKS,  // Get double-clicks
        .lpfnWndProc = win32_process_message,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = state->h_instance,
        .hIcon = LoadIcon(state->h_instance, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = nullptr,  // Transparent
        .lpszClassName = "kohiWindowClass",
    };

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error",
                    MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create window
    u32 client_x {x};
    u32 client_y {y};
    u32 client_width {width};
    u32 client_height {height};

    u32 window_x {client_x};
    u32 window_y {client_y};
    u32 window_width {client_width};
    u32 window_height {client_height};

    u32 window_style {WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX |
                      WS_MINIMIZEBOX | WS_THICKFRAME};
    u32 window_ex_style {WS_EX_APPWINDOW};

    // Obtain the size of the border.
    RECT border_rect {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, the border rectangle is negative.
    window_x += border_rect.left;
    window_y += border_rect.top;

    // Grow by the size of the OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle {CreateWindowExA(window_ex_style, wc.lpszClassName,
                                 application_name, window_style, window_x,
                                 window_y, window_width, window_height, 0, 0,
                                 state->h_instance, 0)};

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);

        KFATAL("Window creation failed!");

        return false;
    } else {
        state->hwnd = handle;
    }

    // Show the window
    bool shouldActivate {true};  // TODO: if the window should not accept input,
                                 // this should be false.
    i32 show_window_command_flags {should_activate ? SW_SHOW
                                                   : SW_SHOWNOACTIVATE};
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(state->hwnd, show_window_command_flags);

    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency {1.0 / (f64)frequency.QuadPart};
    QueryPerformanceCounter(&startTime);

    return true;
}

void PlatformShutdown(PlatformState* platformState) {
    // Simply cold-cast to the known type.
    auto* state {static_cast<InternalState*>(platformState->internalState)};

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

bool PlatformPollMessages(PlatformState* platformState) {
    MSG message {};

    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

void* PlatformAllocate(u64 size, bool aligned) {
    (void)aligned;
    return std::malloc(static_cast<size_t>(size));
}

void PlatformFree(void* block, bool aligned) {
    (void)aligned;
    std::free(block);
}

void* PlatformZeroMemory(void* block, u64 size) {
    return std::memset(block, 0, static_cast<size_t>(size));
}

void* PlatformCopyMemory(void* destination, const void* source, u64 size) {
    return std::memcpy(destination, source, static_cast<size_t>(size));
}

void* PlatformSetMemory(void* destination, const i32 value, u64 size) {
    return std::memset(destination, value, static_cast<size_t>(size));
}

void PlatformConsoleWrite(const char* message, u8 colour) {
    (void)colour;
    std::fputs(message, stdout);
}

void PlatformConsoleWriteError(const char* message, u8 colour) {
    (void)colour;
    std::fputs(message, stderr);
}

f64 PlatformGetAbsoluteTime() {
    using clock {std::chrono::steady_clock};

    const auto now {clock::now().time_since_epoch()};

    return std::chrono::duration<f64>(now).count();
}

void PlatformSleep(u64 milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

LRESULT CALLBACK Win32ProcessMassage(HWND hwnd, u32 msg, WPARAM wparam,
                                     LPARAM lparam) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to
            // prevent flicker.
            return 1;
        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // Get the updated size.
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            // i32 x_position = GET_X_LPARAM(l_param);
            // i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL: {
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // Flatten the input to an OS-independent (-1, 1)
            //     z_delta = (z_delta < 0) ? -1 : 1;
            //     // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            // b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN ||
            // msg == WM_MBUTTONDOWN;
            //  TODO: input processing.
        } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif  // KPLATFORM_WINDOWS