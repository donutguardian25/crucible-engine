#pragma once

// EntryPoint.h — the engine's main().
//
// Include this in EXACTLY ONE .cpp file of your game. Including it twice gives
// you two definitions of main() and a linker error.
//
// ---------------------------------------------------------------------------
// Why the engine owns main() at all (inversion of control)
//
// The obvious arrangement is the opposite: the game writes main(), creates an
// engine, and drives it. That works, and it means every game must independently
// remember to initialise logging before anything logs, start subsystems in the
// right order, and tear them down in the reverse of that order. Get the order
// wrong and you get a crash at shutdown that looks like it came from nowhere.
//
// Inverting it puts that sequence in one place, inside the engine. A game says
// *what* to run — one factory function — and the engine decides *how* to start
// and stop. When Phase 1 adds a window and a graphics context to the startup
// sequence, no game changes at all.
//
// ---------------------------------------------------------------------------
// Why this lives in a header instead of being compiled into the library
//
// A static library is a bag of separately compiled object files, and the linker
// only pulls one out when something already in the link needs a symbol it
// defines. Nothing in your program ever refers to `main` by name — the call
// comes from the C runtime's startup code, outside the set being scanned — so
// an object file containing main() inside a static library is routinely dropped
// and you get "unresolved external symbol main". Putting main() in a header the
// game compiles means it lands in the *game's* object file, where it is always
// present. See docs/decisions.md ADR-003.
// ---------------------------------------------------------------------------

#include "Crucible/Core/Application.h"
#include "Crucible/Core/Assert.h"
#include "Crucible/Core/Log.h"

#include <memory>

int main(int argc, char** argv)
{
    CE_UNUSED(argc);
    CE_UNUSED(argv);

    // Logging comes up first and goes down last, so that every other subsystem
    // — including any that fails during its own startup — has somewhere to
    // report to.
    Crucible::Log::Init();

    CE_CORE_INFO("Crucible Engine initialising");

    // Ownership transfers to the engine the instant the factory returns. The
    // unique_ptr means the game is destroyed correctly even if Run() throws.
    std::unique_ptr<Crucible::Application> app{ Crucible::CreateApplication() };
    CE_CORE_ASSERT(app != nullptr, "CreateApplication() returned nullptr");

    app->Run();

    app.reset();

    CE_CORE_INFO("Crucible Engine shutting down");
    Crucible::Log::Shutdown();

    return 0;
}
