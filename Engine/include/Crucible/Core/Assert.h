#pragma once

// Assert.h — assertions for programmer errors.
//
// An assert states an invariant you believe can never be false. It is not error
// handling: a file that fails to open is a runtime condition to handle, whereas
// a null pointer where the code guarantees one cannot exist is a bug. Asserts
// are for the second kind, which is why they can be safely removed in shipping
// builds — if the invariant can actually be violated at runtime, it was never
// an invariant and needs real error handling instead.

#include "Crucible/Core/Base.h"
#include "Crucible/Core/Log.h"

#if defined(CRUCIBLE_DIST)

    // Compiled out entirely, per CLAUDE.md §6. CE_UNUSED keeps the condition
    // from producing an "unused variable" warning when a local existed solely
    // to be asserted on.
    #define CE_CORE_ASSERT(condition, ...) CE_UNUSED(condition)
    #define CE_ASSERT(condition, ...)      CE_UNUSED(condition)

#else

    #define CE_INTERNAL_ASSERT(channelValue, condition, ...)                       \
        do {                                                                       \
            if (!(condition))                                                      \
            {                                                                      \
                ::Crucible::Log::Write(                                            \
                    ::Crucible::Log::Channel::channelValue,                        \
                    ::Crucible::Log::Level::Critical,                              \
                    ::std::format("Assertion failed: {}\n    {}\n    at {}:{}",    \
                                  #condition,                                      \
                                  ::std::format(__VA_ARGS__),                      \
                                  __FILE__,                                        \
                                  __LINE__));                                      \
                CE_DEBUGBREAK();                                                   \
            }                                                                      \
        } while (false)

    #define CE_CORE_ASSERT(condition, ...) CE_INTERNAL_ASSERT(Core,   condition, __VA_ARGS__)
    #define CE_ASSERT(condition, ...)      CE_INTERNAL_ASSERT(Client, condition, __VA_ARGS__)

#endif
