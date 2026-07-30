# Architecture Decision Record — Crucible Engine

This log records *why* the engine is built the way it is. Each entry states the
decision, what else was on the table, and the reasoning. Append new entries as
work proceeds; do not rewrite history — if a decision is reversed, add a new
entry that supersedes the old one and mark the old one **Superseded**.

Required by `CLAUDE.md` §11.

---

## Inherited constraints (locked in `CLAUDE.md` before this log began)

These were decided in the project spec and are recorded here only so this file
is a complete picture. Reasoning lives in `CLAUDE.md` §2.

| Constraint | Value |
|---|---|
| Primary OS | Windows first |
| Compiler / IDE | MSVC / Visual Studio 2022 |
| Language | C++20, **no** modules |
| Graphics API | OpenGL 4.6 core profile (Phase 1+) |
| Build system | CMake, target-based |
| Dependency fetch | FetchContent |
| Engine artifact | Static library |
| Build configs | Debug / Release / Dist |

---

## ADR-001 — Namespace `Crucible`, macro prefixes `CE_` and `CRUCIBLE_`

**Status:** Accepted — 2026-07-27

**Decision.** All engine code lives in namespace `Crucible`. Macros use two
prefixes by role:

- `CE_` — things written constantly in ordinary code: `CE_CORE_INFO`,
  `CE_TRACE`, `CE_ASSERT`, `CE_CORE_ASSERT`.
- `CRUCIBLE_` — build-configuration and platform flags, which appear rarely and
  benefit from being unmistakable: `CRUCIBLE_DEBUG`, `CRUCIBLE_RELEASE`,
  `CRUCIBLE_DIST`, `CRUCIBLE_PLATFORM_WINDOWS`.

**Alternatives considered.**
- A single prefix everywhere. Consistent, but either verbose at every log call
  (`CRUCIBLE_CORE_INFO`) or cryptic on the flags (`CE_DIST`).
- Keeping the spec's placeholder `Engine` / `ENGINE_`. Rejected: `Engine` is a
  generic name likely to collide, and the project now has a real one.

**Reason.** Prefix length should track how often you type the thing. Log and
assert macros appear on thousands of lines; config flags appear in a handful of
`#if` blocks and CMake definitions where clarity beats brevity.

**Note.** This supersedes the `ENGINE_` naming shown in `CLAUDE.md` §6–§8. See
*Deviations* below.

---

## ADR-002 — Public headers in `include/`, internals in `src/`

**Status:** Accepted — 2026-07-27

**Decision.** The `Engine` target has two header trees:

```
Engine/include/Crucible/   <- the public API; Sandbox and tests may include these
Engine/src/                <- implementation and internal headers; nobody outside may
```

CMake exposes `include/` as `PUBLIC` and `src/` as `PRIVATE`. Consumers
therefore *cannot compile* against an internal header — the include path simply
does not resolve for them.

**Alternatives considered.**
- A single `src/` tree with one `Crucible.h` umbrella header, exactly as drawn
  in `CLAUDE.md` §5. Simpler to navigate and common in tutorial engines. But
  the whole tree has to be exposed for the umbrella to work, so the public/
  private boundary becomes honour-system, enforced only in code review.
- A single tree with per-file export annotations. More machinery than a
  static-library project of this size warrants.

**Reason.** `CLAUDE.md` §3 asks that "the build should resist illegal
dependencies, not just code review", and §1 makes the engine-as-library split a
structural axiom — if `Sandbox` needs an internal header, that is defined as a
hole in the public API. A physical split turns both of those from aspirations
into compiler errors, which is the only form of architectural rule that
survives contact with a deadline.

**Consequences.**
- Moving a type from internal to public is a deliberate act (a file move), which
  is the point — it forces the question "should this be part of the API?"
- Slightly more navigation friction: a class's header and its `.cpp` sit in
  different trees.
- This is a deviation from the folder diagram in `CLAUDE.md` §5. See
  *Deviations* below.

---

## ADR-003 — Entry point via `EntryPoint.h`, included exactly once

**Status:** Accepted — 2026-07-27

**Decision.** The engine provides `main()` inside a public header,
`Crucible/EntryPoint.h`. The game includes that header in exactly one `.cpp`
file and implements the factory `Crucible::Application* Crucible::CreateApplication()`.

**Alternatives considered.** Compiling `main()` directly into the static
library, so the game includes nothing special. Rejected for a concrete linker
reason worth understanding:

> A static library is a bag of independently compiled object files. The linker
> does not add all of them to the program — it adds an object file only when
> something already in the link asks for a symbol that object defines. Nothing
> in the program's own code ever references `main` by name; the reference comes
> from the C runtime's startup stub, which is outside the pieces being scanned.
> So the object holding `main()` is frequently never pulled in, and the build
> fails with `unresolved external symbol main`. The usual escapes —
> `/WHOLEARCHIVE` or a forced `/INCLUDE:` — are MSVC-specific linker flags,
> which cuts against the "stay portable-friendly" instruction in `CLAUDE.md` §9.

**Reason.** The header approach sidesteps that problem entirely: `main()` is
compiled into the *game's* own object file, so it is always present. The cost is
one `#include` of ceremony in the game, which is a fair trade for a link that
behaves identically on every toolchain.

**Reason for the inversion itself** (why the engine owns `main()` at all): it
keeps bootstrap ordering — logger initialisation, subsystem startup, and
teardown sequence — inside the engine, where it can be changed once and take
effect for every game. The game declares *what* to run, not *how* to start.

---

## ADR-004 — Dist builds keep warnings and errors

**Status:** Accepted — 2026-07-27

**Decision.** In the Dist configuration, `TRACE`/`INFO` log macros compile to
nothing; `WARN`, `ERROR` and `CRITICAL` remain live. Assert macros compile out
entirely, per `CLAUDE.md` §6.

**Alternatives considered.** Removing logging altogether in Dist. Smallest and
fastest, and `CLAUDE.md` §2 permits it.

**Reason.** The high-frequency log calls — the ones sitting in frame loops and
costing real time — are all trace and info. Removing those captures nearly all
of the performance and binary-size benefit. Warnings and errors fire rarely by
construction, so retaining them costs approximately nothing and means a shipped
build that misbehaves on someone else's hardware can still say something. A
silent shipping build is a bug report you cannot act on.

---

## ADR-005 — A test target exists from Phase 0

**Status:** Accepted — 2026-07-27

**Decision.** A `Tests` executable using doctest is set up in Phase 0, with a
small number of trivial tests.

**Alternatives considered.** Deferring tests to Phase 1, since the six Phase 0
deliverables in `CLAUDE.md` §9 do not list them.

**Reason.** The tests are not the point yet — the *second consumer* is. With
only `Sandbox` linking the engine, it is easy to accidentally build something
that works solely because of how `Sandbox` happens to be compiled. A second,
independent executable linking the same library proves the engine is genuinely
reusable, which is the structural axiom in `CLAUDE.md` §1. It also means the
"can this be used without a GPU context?" litmus test from §3 has somewhere to
live when assets arrive in later phases.

---

## ADR-006 — Visual Studio multi-config generator

**Status:** Accepted — 2026-07-27

**Decision.** Generate a Visual Studio 2022 solution (`-G "Visual Studio 17 2022"`),
with `Debug`, `Release` and `Dist` as configurations selectable in the IDE
dropdown. `Dist` is added to `CMAKE_CONFIGURATION_TYPES` as a custom config.

**Alternatives considered.** Ninja, which builds noticeably faster. Rejected for
now because Ninja is single-config: each configuration needs its own build
directory and its own configure step, and the debugging workflow through Visual
Studio is less direct. Speed is not the bottleneck at this size.

**Reason.** `CLAUDE.md` §2 fixes Visual Studio 2022 as the environment. A native
solution gives the debugger, profiler and configuration switching that the IDE
is good at, with no extra wiring. Revisit if configure/build times become
annoying.

---

## ADR-007 — Dependencies via FetchContent at pinned tags

**Status:** Accepted — 2026-07-27

**Decision.** Phase 0 pulls exactly two dependencies, each pinned to a specific
release tag rather than a branch:

- **spdlog** — logging backend, wrapped behind our own `Log` facade.
- **doctest** — unit test framework.

No other third-party code enters the build during Phase 0. GLFW, GLAD, GLM,
stb_image and Dear ImGui belong to Phase 1 and later.

**Alternatives considered.** Git submodules (more ceremony, easy to leave in a
detached or stale state) and vcpkg (worth it once the dependency count grows —
`CLAUDE.md` §2 already anticipates that graduation).

**Reason.** Pinning to a tag rather than a branch means a build today and a
build in a year produce the same binaries. A moving branch turns an unrelated
upstream change into a mysterious local breakage.

**Known risk.** The toolchain here is CMake 4.4, and CMake 4 removed support for
projects that declare a `cmake_minimum_required` below 3.5. Some dependency
releases still declare very old minimums and fail to configure as a result. If
that happens, the fix order is: (1) move to a newer upstream tag that has
corrected its minimum, and only failing that (2) set
`CMAKE_POLICY_VERSION_MINIMUM` for that dependency's scope. Record the outcome
here if it occurs.

---

## ADR-008 — spdlog keeps its bundled formatting library

**Status:** Accepted — 2026-07-27

**Decision.** Use spdlog's default bundled formatter rather than switching it to
the C++20 standard library formatter via `SPDLOG_USE_STD_FORMAT`.

**Alternatives considered.** The standard formatter, which would drop one
vendored dependency from the build.

**Reason.** The bundled path is spdlog's default and best-tested configuration,
and it behaves identically across compilers — whereas standard-library
formatting support still varies between MSVC, libstdc++ and libc++, which
matters once Linux and macOS arrive. This is a cheap decision to reverse later;
it is a single CMake option.

---

## ADR-009 — The Log facade is fully opaque; spdlog never appears in a public header

**Status:** Accepted — 2026-07-27

**Decision.** `Crucible/Core/Log.h` includes only `<format>` and
`<string_view>`. It declares an enum for level, an enum for channel, and three
functions (`Write`, `IsEnabled`, plus `Init`/`Shutdown`). The log macros format
with `std::format` at the call site and hand the finished string to `Write`.
spdlog is included exactly once in the whole project, in `Log.cpp`, and is
linked `PRIVATE`.

**Alternatives considered.** The common approach — used by most tutorial engines
— is to expose the spdlog logger objects from `Log.h` and have the macros call
straight into spdlog. That is faster (spdlog formats lazily and can skip work
the facade cannot) and it is less code. It also makes spdlog a `PUBLIC`
dependency: every game and every tool that touches a log macro now compiles
spdlog's headers, and the "backend" is no longer replaceable without touching
every call site.

**Reason.** `CLAUDE.md` §4 describes spdlog as a backend "wrapped behind our own
`Log` facade", and §11 requires vendor libraries stay out of public headers.
A facade that re-exports the thing it wraps is not a facade. Keeping it opaque
means swapping the backend later — for an in-engine ring-buffer logger feeding
an ImGui console window, which is a very likely Phase 2/3 want — is a change to
one `.cpp` file and one CMake line.

**Consequences.**
- The macros call `IsEnabled()` before formatting, so a disabled log call costs
  one function call rather than a full string construction. This matters: the
  cost of a *disabled* trace call in a frame loop is entirely the formatting
  that gets thrown away.
- Formatting happens eagerly once the level check passes, where spdlog would
  defer it. For a Phase 0 engine this is immeasurable; revisit if profiling ever
  says otherwise.
- `std::format` requires a compile-time format string, so a malformed one is a
  compile error rather than a runtime surprise. That is an improvement.

**Verified.** The generated `Sandbox.vcxproj` lists exactly one include
directory, `Engine/include`. No spdlog header and no `Engine/src` path is
reachable from the game.

**Note on static libraries.** spdlog *does* still appear on Sandbox's link line
as `spdlogd.lib`, and this is unavoidable rather than a leak. A static library
is not self-contained: it does not absorb the libraries it uses, so anything
linking `Engine` must also link what `Engine` needed. `PRIVATE` controls what
propagates at the **source** level — include paths and compile definitions —
and that boundary holds completely. Link-level propagation is a property of
static linking itself, and would only disappear if the engine became a DLL,
which `CLAUDE.md` §2 explicitly rejects for other reasons.

---

## Deliberate technical debt

Recorded so it is a decision rather than an oversight. Each entry needs a
trigger describing when to revisit.

### TD-001 — No renderer abstraction layer

**Taken on:** 2026-07-27 (declared in `CLAUDE.md` §3, recorded here)

The renderer will be written as straightforward OpenGL wrapper classes
(`Shader`, `Texture2D`, `VertexBuffer`, `VertexArray`) with no API-agnostic
abstraction beneath them.

**Why deliberately.** An abstraction designed before you understand the second
implementation encodes the assumptions of the first. Building one now would
produce an OpenGL-shaped interface that Vulkan cannot fit, and the refactor
would be worse than having written nothing. Feeling the seams first is the
cheaper path to a correct abstraction.

**Trigger to revisit.** When a Vulkan backend is genuinely being started
(Phase 6+), not before.

---

## Deviations from `CLAUDE.md`

Flagged per `CLAUDE.md` §11, which requires deviations be surfaced rather than
silently adopted. Both were proposed and confirmed before any code was written.

| Spec section | What it says | What we do | Where |
|---|---|---|---|
| §5 | Single `Engine/src/` tree | Split `Engine/include/Crucible/` + `Engine/src/` | ADR-002 |
| §6–§8 | Macro prefix `ENGINE_` | `CE_` for common macros, `CRUCIBLE_` for build flags | ADR-001 |

`CLAUDE.md` has not been edited to match; this table is the reconciliation. If
the spec is ever updated, remove the rows that no longer differ.
