#pragma once

// Base.h — foundational macros shared by every other public header.
//
// PUBLIC header: games compile against this. It may include the standard
// library and nothing else. No spdlog, no GLFW, no engine internals.

// ---------------------------------------------------------------------------
// Build configuration
//
// CMake defines exactly one of these (see Engine/CMakeLists.txt). Checking here
// turns "someone linked the engine without our compile definitions" from a
// subtle behavioural mismatch — where the game's log macros strip differently
// than the engine's did — into a compile error on line one.
// ---------------------------------------------------------------------------
#if !defined(CRUCIBLE_DEBUG) && !defined(CRUCIBLE_RELEASE) && !defined(CRUCIBLE_DIST)
    #error "No Crucible build configuration defined. Link against the Engine target so CMake supplies CRUCIBLE_DEBUG / CRUCIBLE_RELEASE / CRUCIBLE_DIST."
#endif

// ---------------------------------------------------------------------------
// Debug break
//
// Halts the program *inside the debugger at the offending line*, rather than
// unwinding or aborting. That distinction is the whole value: when an assert
// fires you want the live stack, locals and registers still intact so you can
// inspect them, not a post-mortem core dump with the frames already gone.
// ---------------------------------------------------------------------------
#if defined(_MSC_VER)
    #define CE_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define CE_DEBUGBREAK() __builtin_trap()
#else
    #include <cstdlib>
    #define CE_DEBUGBREAK() ::std::abort()
#endif

// Marks a value as deliberately unused. In Dist builds the assert macros
// compile away, which can strand a variable that existed only to be checked;
// this silences the resulting warning without disabling the warning globally.
#define CE_UNUSED(x) ((void)(x))

// Expands a macro argument before stringifying it. Needed because the
// preprocessor otherwise stringifies the argument's *name* rather than its
// value — the classic two-step stringify idiom.
#define CE_STRINGIFY_IMPL(x) #x
#define CE_STRINGIFY(x)      CE_STRINGIFY_IMPL(x)
