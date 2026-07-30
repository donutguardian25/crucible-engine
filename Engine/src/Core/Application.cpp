#include "crpch.h"

#include "Crucible/Core/Application.h"
#include "Crucible/Core/Log.h"

namespace Crucible {

Application::Application()
{
    CE_CORE_TRACE("Application constructed");
}

Application::~Application()
{
    CE_CORE_TRACE("Application destroyed");
}

void Application::Run()
{
    CE_CORE_INFO("Entering main loop");

    // Phase 0 has no window and no events, so there is nothing to keep the loop
    // alive and nothing to end it. Phase 1 replaces this with a real loop
    // driven by window events; for now, one pass proves the lifecycle works
    // end to end.
    while (m_Running)
    {
        m_Running = false;
    }

    CE_CORE_INFO("Exiting main loop");
}

} // namespace Crucible
