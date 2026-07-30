# ---------------------------------------------------------------------------
# Engine — the static library everything else links against.
# ---------------------------------------------------------------------------

add_library(Engine STATIC
    # Public headers are listed so they show up in the IDE; they are not compiled.
    include/Crucible.h
    include/Crucible/EntryPoint.h
    include/Crucible/Core/Base.h
    include/Crucible/Core/Log.h
    include/Crucible/Core/Assert.h
    include/Crucible/Core/Application.h

    src/crpch.h
    src/crpch.cpp
    src/Core/Log.cpp
    src/Core/Application.cpp
)

add_library(Crucible::Engine ALIAS Engine)

# ---------------------------------------------------------------------------
# The public/private boundary — see docs/decisions.md ADR-002.
#
# PUBLIC  include/ : anything a game is allowed to use. Propagates to Sandbox.
# PRIVATE src/     : implementation and internal headers. Does NOT propagate, so
#                    a game that tries to #include an internal header simply
#                    fails to compile. The build enforces the layering rather
#                    than relying on code review to catch it.
# ---------------------------------------------------------------------------
target_include_directories(Engine
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(Engine PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# Build-configuration and platform macros.
#
# These are PUBLIC because the public headers use them: the log and assert
# macros in Log.h / Assert.h check CRUCIBLE_DIST to decide whether to compile
# away. If a game were built without these defined, its macros would disagree
# with the ones the engine was compiled with.
# ---------------------------------------------------------------------------
target_compile_definitions(Engine PUBLIC
    $<$<CONFIG:Debug>:CRUCIBLE_DEBUG>
    $<$<CONFIG:Release>:CRUCIBLE_RELEASE>
    $<$<CONFIG:Dist>:CRUCIBLE_DIST>
    $<$<PLATFORM_ID:Windows>:CRUCIBLE_PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Linux>:CRUCIBLE_PLATFORM_LINUX>
    $<$<PLATFORM_ID:Darwin>:CRUCIBLE_PLATFORM_MACOS>
)

# ---------------------------------------------------------------------------
# spdlog is linked PRIVATE: it is an implementation detail of our Log facade.
# Nothing outside this library ever sees a spdlog header or links against it
# directly. Swapping the backend later touches Log.cpp and this line only.
# ---------------------------------------------------------------------------
target_link_libraries(Engine PRIVATE spdlog::spdlog)

# ---------------------------------------------------------------------------
# Precompiled header.
#
# PRIVATE on purpose. A PCH is a compile-time speed trick for *this* library's
# own .cpp files; forcing it on consumers would make their builds depend on our
# internal include list. It also means every public header must stand on its own
# — if a public header only compiles because the PCH happened to include
# <string>, that is a bug, and keeping the PCH private is what exposes it.
# ---------------------------------------------------------------------------
target_precompile_headers(Engine PRIVATE src/crpch.h)

if(MSVC)
    target_compile_options(Engine PRIVATE /W4 /permissive-)
    # Give the IDE the same folder layout as the filesystem.
    get_target_property(engineSources Engine SOURCES)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${engineSources})
else()
    target_compile_options(Engine PRIVATE -Wall -Wextra)
endif()

set_target_properties(Engine PROPERTIES FOLDER "Crucible")
