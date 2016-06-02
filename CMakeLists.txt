cmake_minimum_required (VERSION 2.8.7)

project (walls-of-doom)

set (walls-of-doom_VERSION_MAJOR 0)
set (walls-of-doom_VERSION_MINOR 6)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# The standard is C 90, always.
set (C_STANDARD 90)

set (CMAKE_BUILD_TYPE Release)

# Based on which compiler we are using, set warning flags
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_C_FLAGS "-Weverything")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set (CMAKE_C_FLAGS "-Wall -Wextra")
endif ()

add_subdirectory (source)
add_subdirectory (unity)
add_subdirectory (autotest)
add_subdirectory (game)
