cmake_minimum_required(VERSION 3.24)

project(Crucible
    VERSION 0.1.0
    DESCRIPTION "Crucible — a C++20 game engine built from scratch"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Build configurations
#
# CMake ships Debug/Release/RelWithDebInfo/MinSizeRel. We want exactly three:
# Debug, Release and a custom "Dist" (shipping) configuration.
#
# A multi-config generator — which the Visual Studio generator is — bakes the
# configuration list into the generated solution, so this must be set before any
# target is defined. Every custom configuration also needs its own set of
# compiler and linker flag variables; CMake does not invent them for you, so we
# seed Dist from Release and let the engine's own macros do the stripping.
# ---------------------------------------------------------------------------
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)
else()
    set(allowedBuildTypes Debug Release Dist)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type '${CMAKE_BUILD_TYPE}'. Use one of: ${allowedBuildTypes}")
    endif()
endif()

set(CMAKE_CXX_FLAGS_DIST           "${CMAKE_CXX_FLAGS_RELEASE}"           CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DIST    "${CMAKE_EXE_LINKER_FLAGS_RELEASE}"    CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DIST "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_DIST "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_DIST
    CMAKE_EXE_LINKER_FLAGS_DIST
    CMAKE_SHARED_LINKER_FLAGS_DIST
    CMAKE_STATIC_LINKER_FLAGS_DIST
)

# Dependencies fetched below know nothing about a configuration called "Dist".
# This tells CMake to use their Release artifacts when we build Dist, instead of
# failing to find a "Dist" build of spdlog that was never produced.
set(CMAKE_MAP_IMPORTED_CONFIG_DIST Release "")

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++20, not gnu++20: keeps us portable

# One runtime library choice across the whole build. Mixing the debug and
# release CRT within a single program is a classic Windows crash — allocations
# made by one heap freed by the other.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Put every binary in one predictable place, per configuration.
foreach(config Debug Release Dist)
    string(TOUPPER ${config} configUpper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/bin/${config}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/bin/${config}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/lib/${config}")
endforeach()

# Visual Studio niceties: mirror the folder layout in the solution explorer, and
# start debugging in Sandbox rather than whichever target CMake picks first.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# Third-party dependencies
#
# Pinned to specific release tags, never a branch: a build today and a build in
# a year should produce identical binaries. See docs/decisions.md ADR-007.
# ---------------------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

# spdlog — logging backend. Kept PRIVATE to the Engine target; no game or test
# code ever sees a spdlog header. See docs/decisions.md ADR-009.
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL       OFF CACHE BOOL "" FORCE)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
    GIT_SHALLOW    TRUE
)

# doctest — unit tests. Chosen for compile speed. See docs/decisions.md ADR-005.
set(DOCTEST_WITH_TESTS               OFF CACHE BOOL "" FORCE)
set(DOCTEST_NO_INSTALL               ON  CACHE BOOL "" FORCE)
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.12
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(spdlog doctest)

# Keep vendored targets out of the way in the IDE.
foreach(vendorTarget spdlog doctest doctest_with_main)
    if(TARGET ${vendorTarget})
        set_target_properties(${vendorTarget} PROPERTIES FOLDER "vendor")
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Our targets
# ---------------------------------------------------------------------------
add_subdirectory(Engine)
add_subdirectory(Sandbox)

include(CTest)
add_subdirectory(tests)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
