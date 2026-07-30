// CoreTests.cpp — Phase 0 sanity tests.
//
// DOCTEST_CONFIG_IMPLEMENT (rather than ..._WITH_MAIN) lets us write our own
// main, which is the point: this executable links the engine but supplies its
// own entry point instead of taking the engine's. Proving the engine works
// without EntryPoint.h is what makes it a library rather than a framework.

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <Crucible.h>

using Crucible::Log;

TEST_CASE("Log::Init is idempotent")
{
    // Calling twice must not duplicate sinks or crash. The engine's main()
    // calls it once, but tests and tools may not be so disciplined.
    Log::Init();
    Log::Init();

    CHECK(Log::IsEnabled(Log::Channel::Core, Log::Level::Critical));
}

TEST_CASE("Both channels accept messages at every level")
{
    Log::Init();

    CE_CORE_TRACE("core trace {}", 1);
    CE_CORE_INFO("core info {}", 2);
    CE_CORE_WARN("core warn {}", 3);
    CE_CORE_ERROR("core error {}", 4);

    CE_TRACE("client trace {}", 5);
    CE_INFO("client info {}", 6);
    CE_WARN("client warn {}", 7);
    CE_ERROR("client error {}", 8);

    // Reaching this line at all is the assertion — the checks above are for
    // crashes and format-string errors, neither of which returns a value.
    CHECK(Log::IsEnabled(Log::Channel::Core, Log::Level::Critical));
}

TEST_CASE("Runtime log level follows the build configuration")
{
    Log::Init();

    // Warnings and above are enabled in every configuration, Dist included.
    // This is the guarantee ADR-004 rests on.
    CHECK(Log::IsEnabled(Log::Channel::Core,   Log::Level::Warn));
    CHECK(Log::IsEnabled(Log::Channel::Client, Log::Level::Error));

#if defined(CRUCIBLE_DEBUG)
    CHECK(Log::IsEnabled(Log::Channel::Core, Log::Level::Trace));
#elif defined(CRUCIBLE_RELEASE)
    CHECK(Log::IsEnabled(Log::Channel::Core, Log::Level::Info));
    CHECK_FALSE(Log::IsEnabled(Log::Channel::Core, Log::Level::Trace));
#elif defined(CRUCIBLE_DIST)
    CHECK_FALSE(Log::IsEnabled(Log::Channel::Core, Log::Level::Info));
    CHECK_FALSE(Log::IsEnabled(Log::Channel::Core, Log::Level::Trace));
#endif
}

namespace {

class TestApp : public Crucible::Application
{
public:
    // m_Running is protected in Application, so a derived class is the only way
    // to observe it — which is exactly how a real game would reach it.
    [[nodiscard]] bool IsRunning() const { return m_Running; }
};

// Defeats constant folding so the value below is not known at compile time.
// See the comment in the assert test for why that matters.
[[nodiscard]] int Opaque(int value) { return value; }

} // namespace

TEST_CASE("Application runs and exits cleanly")
{
    Log::Init();

    TestApp app;
    CHECK(app.IsRunning());

    app.Run();
    CHECK_FALSE(app.IsRunning());

    // A second call must terminate immediately rather than spin.
    app.Run();
    CHECK_FALSE(app.IsRunning());
}

TEST_CASE("Asserts pass silently when their condition holds")
{
    Log::Init();

    // Deliberately routed through Opaque(). Asserting on a compile-time
    // constant makes MSVC emit C4127 ("conditional expression is constant")
    // because the check folds away entirely — and a condition the compiler can
    // already answer is not exercising the assert macro the way real code does.
    const int value = Opaque(42);

    CE_CORE_ASSERT(value == 42, "arithmetic still works");
    CE_ASSERT(value > 0, "value should be positive, was {}", value);

    CHECK(value == 42);
}

int main(int argc, char** argv)
{
    Crucible::Log::Init();

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    const int result = context.run();

    Crucible::Log::Shutdown();
    return result;
}
