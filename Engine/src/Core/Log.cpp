#include "crpch.h"

#include "Crucible/Core/Log.h"

// spdlog appears here and nowhere else in the project. Everything above this
// line in the dependency graph talks to Crucible::Log instead.
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <cstdio>

namespace Crucible {

namespace {

    std::shared_ptr<spdlog::logger> g_CoreLogger;
    std::shared_ptr<spdlog::logger> g_ClientLogger;

    constexpr spdlog::level::level_enum ToSpdlogLevel(Log::Level level)
    {
        switch (level)
        {
            case Log::Level::Trace:    return spdlog::level::trace;
            case Log::Level::Info:     return spdlog::level::info;
            case Log::Level::Warn:     return spdlog::level::warn;
            case Log::Level::Error:    return spdlog::level::err;
            case Log::Level::Critical: return spdlog::level::critical;
        }
        return spdlog::level::info;
    }

    spdlog::logger* Select(Log::Channel channel)
    {
        return channel == Log::Channel::Core ? g_CoreLogger.get() : g_ClientLogger.get();
    }

    // Threshold per build configuration.
    //
    // Note this is a *runtime* filter and is separate from the compile-time
    // stripping the macros in Log.h perform. The macros decide what code exists
    // at all; this decides what the surviving calls actually emit. Dist has
    // both: trace/info are gone entirely, and the backend is set to warn as a
    // second line of defence.
    constexpr spdlog::level::level_enum DefaultLevel()
    {
#if defined(CRUCIBLE_DEBUG)
        return spdlog::level::trace;
#elif defined(CRUCIBLE_RELEASE)
        return spdlog::level::info;
#else // CRUCIBLE_DIST
        return spdlog::level::warn;
#endif
    }

} // namespace

void Log::Init()
{
    if (g_CoreLogger)
        return; // idempotent: calling twice should not duplicate sinks

    // %^ and %$ bracket the region the console sink colours by severity.
    //   %T   wall-clock time
    //   %e   milliseconds — frame-level work needs sub-second resolution
    //   %l   level name
    //   %n   logger name, i.e. which channel
    const char* pattern = "%^[%T.%e] [%l] %n: %v%$";

    g_CoreLogger = spdlog::stdout_color_mt("CRUCIBLE");
    g_CoreLogger->set_pattern(pattern);
    g_CoreLogger->set_level(DefaultLevel());

    g_ClientLogger = spdlog::stdout_color_mt("APP");
    g_ClientLogger->set_pattern(pattern);
    g_ClientLogger->set_level(DefaultLevel());

    // Flush anything at warn or above immediately. The default buffers, and a
    // buffered log is worthless during a crash — the message describing what
    // went wrong is exactly the one still sitting in memory when the process
    // dies.
    g_CoreLogger->flush_on(spdlog::level::warn);
    g_ClientLogger->flush_on(spdlog::level::warn);
}

void Log::Shutdown()
{
    // Drop our references and let spdlog release its own registry entries.
    // Explicit rather than relying on static destruction order, which is not
    // guaranteed across translation units.
    if (g_CoreLogger)
        g_CoreLogger->flush();
    if (g_ClientLogger)
        g_ClientLogger->flush();

    g_CoreLogger.reset();
    g_ClientLogger.reset();
    spdlog::drop_all();
}

void Log::Write(Channel channel, Level level, std::string_view message)
{
    spdlog::logger* logger = Select(channel);

    if (!logger)
    {
        // Logging before Init() or after Shutdown(). Rather than crash — which
        // would be a miserable failure mode for a diagnostic system — fall back
        // to stderr so the message is not lost.
        std::fprintf(stderr, "[log-not-initialised] %.*s\n",
                     static_cast<int>(message.size()), message.data());
        return;
    }

    logger->log(ToSpdlogLevel(level), message);
}

bool Log::IsEnabled(Channel channel, Level level)
{
    const spdlog::logger* logger = Select(channel);

    // Uninitialised reports as enabled so the stderr fallback above still runs
    // and the message surfaces somewhere.
    if (!logger)
        return true;

    return logger->should_log(ToSpdlogLevel(level));
}

} // namespace Crucible
