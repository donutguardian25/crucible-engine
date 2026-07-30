// SandboxApp.cpp — the entire surface area a game has to implement.
//
// This is the Phase 0 deliverable in miniature: no window, no rendering, just
// proof that the engine boots, logs through both channels, runs an application,
// and shuts down cleanly.

#include <Crucible.h>

// Defines main(). Exactly one file in the game may include this.
#include <Crucible/EntryPoint.h>

namespace Sandbox {

class SandboxApp : public Crucible::Application
{
public:
    SandboxApp()
    {
        CE_INFO("Sandbox constructed");
        CE_TRACE("Client channel is live — this line is absent in Dist builds");
        CE_WARN("Client warnings survive into Dist builds");
    }

    ~SandboxApp() override
    {
        CE_INFO("Sandbox destroyed");
    }
};

} // namespace Sandbox

// The factory the engine calls. This is the whole contract: the engine knows
// how to obtain an Application without knowing which one it is getting.
Crucible::Application* Crucible::CreateApplication()
{
    return new Sandbox::SandboxApp();
}
