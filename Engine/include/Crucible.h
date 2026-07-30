#pragma once

// Crucible.h — the umbrella header a game includes to get the engine's API.
//
// This file is for GAMES ONLY. Engine code never includes it: internal code
// includes precisely the headers it needs, so that touching one header does not
// trigger a rebuild of the entire library.
//
// Note that EntryPoint.h is deliberately absent. It defines main(), so it is
// included separately and in exactly one translation unit of the game.

#include "Crucible/Core/Application.h"
#include "Crucible/Core/Assert.h"
#include "Crucible/Core/Base.h"
#include "Crucible/Core/Log.h"
