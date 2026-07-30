#pragma once

// Log.h — the engine's logging facade.
//
// PUBLIC header. Note what it does NOT include: spdlog. spdlog is the backend,
// and it appears only inside Log.cpp. Games and tests therefore compile against
// the standard library and this declaration alone, which means replacing the
// backend later is a change to one .cpp file and one CMake line — not a
// recompile of every call site in every game. See docs/decisions.md ADR-009.

#include "Crucible/Core/Base.h"

#include <format>
#include <string_view>

namespace Crucible {

class Log
{
public:
    enum class Level
    {
        Trace,
        Info,
        Warn,
        Error,
        Critical
    };

    // Two channels so engine chatter and game chatter can be filtered
    // separately — when the renderer is spewing at Trace, you still want to
    // read your own gameplay logs.
    enum class Channel
    {
        Core,   // the engine itself
        Client  // the game built on top of it
    };

    // Must be called before any logging. The engine's main() does this for you;
    // see EntryPoint.h.
    static void Init();
    static void Shutdown();

    static void Write(Channel channel, Level level, std::string_view message);

    // Cheap level test. The macros below call this *before* formatting, because
    // building a string only to discard it is the entire cost of a disabled log
    // call — and in a frame loop that cost is paid tens of thousands of times a
    // second for output nobody will ever read.
    [[nodiscard]] static bool IsEnabled(Channel channel, Level level);
};

} // namespace Crucible

// ---------------------------------------------------------------------------
// Macro layer
//
// Macros rather than functions for two reasons: only a macro can compile itself
// out of existence in Dist builds, and only a macro can skip evaluating its own
// arguments when the level is disabled.
//
// The do/while(false) wrapper makes the macro behave like a single statement,
// so `if (x) CE_CORE_INFO("hi"); else ...` parses the way you expect instead of
// silently attaching the else to the wrong branch.
// ---------------------------------------------------------------------------
#define CE_INTERNAL_LOG(channelValue, levelValue, ...)                            \
    do {                                                                          \
        constexpr auto ce_channel_ = ::Crucible::Log::Channel::channelValue;      \
        constexpr auto ce_level_   = ::Crucible::Log::Level::levelValue;          \
        if (::Crucible::Log::IsEnabled(ce_channel_, ce_level_))                    \
            ::Crucible::Log::Write(ce_channel_, ce_level_, ::std::format(__VA_ARGS__)); \
    } while (false)

#if defined(CRUCIBLE_DIST)

    // Shipping build: trace and info vanish at compile time — they are the
    // high-frequency calls sitting in hot loops, so removing them captures
    // nearly all of the benefit. Warn/Error/Critical survive, because a shipped
    // build that fails silently on a player's machine is a bug report you
    // cannot act on. See docs/decisions.md ADR-004.
    #define CE_CORE_TRACE(...) ((void)0)
    #define CE_CORE_INFO(...)  ((void)0)
    #define CE_TRACE(...)      ((void)0)
    #define CE_INFO(...)       ((void)0)

#else

    #define CE_CORE_TRACE(...) CE_INTERNAL_LOG(Core,   Trace, __VA_ARGS__)
    #define CE_CORE_INFO(...)  CE_INTERNAL_LOG(Core,   Info,  __VA_ARGS__)
    #define CE_TRACE(...)      CE_INTERNAL_LOG(Client, Trace, __VA_ARGS__)
    #define CE_INFO(...)       CE_INTERNAL_LOG(Client, Info,  __VA_ARGS__)

#endif

#define CE_CORE_WARN(...)     CE_INTERNAL_LOG(Core,   Warn,     __VA_ARGS__)
#define CE_CORE_ERROR(...)    CE_INTERNAL_LOG(Core,   Error,    __VA_ARGS__)
#define CE_CORE_CRITICAL(...) CE_INTERNAL_LOG(Core,   Critical, __VA_ARGS__)

#define CE_WARN(...)          CE_INTERNAL_LOG(Client, Warn,     __VA_ARGS__)
#define CE_ERROR(...)         CE_INTERNAL_LOG(Client, Error,    __VA_ARGS__)
#define CE_CRITICAL(...)      CE_INTERNAL_LOG(Client, Critical, __VA_ARGS__)
