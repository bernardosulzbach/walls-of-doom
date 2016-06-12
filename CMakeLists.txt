cmake_minimum_required (VERSION 2.8.7)

project (walls-of-doom)

set (walls-of-doom_VERSION_MAJOR 0)
set (walls-of-doom_VERSION_MINOR 7)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set (CMAKE_BUILD_TYPE Release)

# C standard is enforced with compiler-specific flags.
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_C_FLAGS "-std=c89")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set (CMAKE_C_FLAGS "-std=c89 -Wpedantic -Wall -Wextra")
endif ()

add_subdirectory (unity)
add_subdirectory (tests)
add_subdirectory (game)
