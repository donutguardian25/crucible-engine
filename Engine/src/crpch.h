#pragma once

// crpch.h — the engine's precompiled header.
//
// A PCH is a compile-time optimisation, nothing more. The compiler parses these
// headers once, snapshots the resulting state to disk, and reuses that snapshot
// for every .cpp file in this library instead of re-parsing tens of thousands
// of lines each time.
//
// INTERNAL header: this lives under src/, which is PRIVATE to the Engine
// target, so no game can include it. That is deliberate. If the PCH were forced
// on consumers, a public header could quietly compile only because the PCH
// happened to include <string> — and the day someone included that header on
// its own, it would break. Keeping the PCH private guarantees every public
// header stands on its own.
//
// Rule of thumb for what belongs here: headers that are large, included widely,
// and change rarely. Adding a header that changes often defeats the purpose,
// because every edit invalidates the snapshot and forces a full rebuild.

// --- Standard library -------------------------------------------------------
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// --- Platform ---------------------------------------------------------------
#if defined(CRUCIBLE_PLATFORM_WINDOWS)
    // Trim the Windows headers hard. Without NOMINMAX, <Windows.h> defines
    // min/max as macros that collide with std::min/std::max and produce
    // spectacularly unhelpful template errors.
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#endif
