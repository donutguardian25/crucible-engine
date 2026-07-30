// Deliberately near-empty. CMake's target_precompile_headers drives PCH
// generation, but keeping a translation unit that includes the PCH means the
// header is still compiled and syntax-checked even if the PCH mechanism is ever
// disabled.
#include "crpch.h"
