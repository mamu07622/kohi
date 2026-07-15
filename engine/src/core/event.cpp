#include "core/event.h"

#include <array>

#include "containers/darray.h"

namespace {

struct RegisteredEvent {
    void* Listener;
    PFN_on_event Callback;
};

struct EventCodeEntry {
    RegisteredEvent* Events {nullptr};
};

constexpr u16 MaxMessageCodes {16384};

struct EventSystemState {
    std::array<EventCodeEntry, MaxMessageCodes> Registered {};
};

}  // namespace

/**
 * Event system internal state.
 */
static bool sIsInitialised {false};
static EventSystemState sEventState {};

bool InitialiseEvent() {
    if (sIsInitialised) {
        return false;
    }

    sIsInitialised = true;

    return true;
}

void ShutdownEvent() {
    // Free the events arrays. Objects pointed to should be destroyed by their
    // owners.
    for (auto& registeredCode : sEventState.Registered) {
        if (registeredCode.Events != nullptr) {
            DArrayDestroy(registeredCode.Events);
            registeredCode.Events = nullptr;
        }
    }

    sEventState = {};
    sIsInitialised = false;
}

bool EventRegister(u16 code, void* listener, PFN_on_event onEvent) {
    if (!sIsInitialised) {
        return false;
    }

    if (sEventState.Registered[code].Events == nullptr) {
        sEventState.Registered[code].Events = DArrayCreate<RegisteredEvent>();
    }

    const u64 registeredCount {
        DArrayLength(sEventState.Registered[code].Events)};

    for (u64 i = 0; i < registeredCount; ++i) {
        if (sEventState.Registered[code].Events[i].Listener == listener) {
            // TODO: warn
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    const RegisteredEvent event {listener, onEvent};

    DArrayPush(sEventState.Registered[code].Events, event);

    return true;
}

bool EventUnregister(u16 code, void* listener, PFN_on_event onEvent) {
    if (!sIsInitialised) {
        return false;
    }

    // On nothing is registered for the code, boot out.
    if (sEventState.Registered[code].Events == nullptr) {
        // TODO: warn
        return false;
    }

    const u64 registeredCount {
        DArrayLength(sEventState.Registered[code].Events)};

    for (u64 i = 0; i < registeredCount; ++i) {
        const RegisteredEvent& event {sEventState.Registered[code].Events[i]};

        if (event.Listener == listener && event.Callback == onEvent) {
            // Found one, remove it
            RegisteredEvent poppedEvent {};

            sEventState.Registered[code].Events = DArrayPopAt(
                sEventState.Registered[code].Events, i, &poppedEvent);

            return true;
        }
    }

    // Not found.
    return false;
}

bool EventFire(u16 code, void* sender, EventContext context) {
    if (!sIsInitialised) {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if (sEventState.Registered[code].Events == nullptr) {
        return false;
    }

    const u64 registeredCount {
        DArrayLength(sEventState.Registered[code].Events)};

    for (u64 i = 0; i < registeredCount; ++i) {
        const RegisteredEvent& event {sEventState.Registered[code].Events[i]};

        if (event.Callback(code, sender, event.Listener, context)) {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}