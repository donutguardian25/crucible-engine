# ---------------------------------------------------------------------------
# Sandbox — the test game. A separate executable that links the engine.
#
# Note what is absent: any reference to Engine/src, to spdlog, or to any
# include directory beyond what the Engine target hands over. Everything this
# target needs to compile arrives through target_link_libraries, because the
# Engine target declares its own public interface. If a line ever has to be
# added here to make Sandbox build, that is a hole in the engine's public API —
# flag it rather than patching it here. (CLAUDE.md §1)
# ---------------------------------------------------------------------------

add_executable(Sandbox
    src/SandboxApp.cpp
)

target_link_libraries(Sandbox PRIVATE Crucible::Engine)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /W4 /permissive-)
    # Debug from the project root so relative asset paths behave the same as
    # they will for a shipped build.
    set_target_properties(Sandbox PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
else()
    target_compile_options(Sandbox PRIVATE -Wall -Wextra)
endif()

set_target_properties(Sandbox PROPERTIES FOLDER "Crucible")
