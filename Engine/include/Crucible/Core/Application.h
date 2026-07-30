#pragma once

// Application.h — the base class every game derives from.
//
// Phase 0 keeps this deliberately empty of subsystems: no window, no renderer,
// no layer stack. Those arrive in Phase 1 and land here as members with a
// well-defined startup and shutdown order. The point right now is the *shape* —
// the engine owns the lifecycle, the game fills in behaviour.

#include "Crucible/Core/Base.h"

namespace Crucible {

class Application
{
public:
    Application();

    // Virtual because the engine deletes games through this base pointer (see
    // EntryPoint.h). Without it, a derived class's destructor never runs and
    // whatever the game owns silently leaks.
    virtual ~Application();

    // Non-copyable, non-movable: an Application owns process-wide resources —
    // eventually a window and a graphics context — that cannot meaningfully be
    // duplicated. Deleting these makes the mistake a compile error rather than
    // a double-free at shutdown.
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&)                 = delete;
    Application& operator=(Application&&)      = delete;

    // The main loop. Phase 1 replaces the body with real windowing and event
    // pumping; the signature stays.
    void Run();

protected:
    bool m_Running = true;
};

// Implemented by the game, called by the engine's main(). This declaration is
// the entire contract between the two: the engine knows a game exists and how
// to construct one, without knowing anything about which game it is.
//
// Returns a raw pointer rather than a unique_ptr purely so the factory a game
// author writes stays a one-liner; the engine takes ownership immediately and
// wraps it — see EntryPoint.h.
Application* CreateApplication();

} // namespace Crucible
