# ---------------------------------------------------------------------------
# Tests — the engine's second consumer.
#
# The tests themselves are trivial right now; their structural job is more
# important. With only Sandbox linking the engine, it is easy to build something
# that works purely because of how Sandbox happens to be compiled. A second,
# independent executable linking the same library proves the engine is genuinely
# reusable — the structural axiom in CLAUDE.md §1. See docs/decisions.md ADR-005.
#
# Note this target does NOT include Crucible/EntryPoint.h: doctest supplies its
# own main(). That the engine can be linked *without* taking its main() is a
# property worth having and worth testing.
# ---------------------------------------------------------------------------

add_executable(Tests
    src/CoreTests.cpp
)

target_link_libraries(Tests PRIVATE
    Crucible::Engine
    doctest::doctest
)

if(MSVC)
    target_compile_options(Tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(Tests PRIVATE -Wall -Wextra)
endif()

set_target_properties(Tests PROPERTIES FOLDER "Crucible")

add_test(NAME CoreTests COMMAND Tests)
