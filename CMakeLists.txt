cmake_minimum_required (VERSION 2.8.7)

project (walls-of-doom)

set (walls-of-doom_VERSION_MAJOR 0)
set (walls-of-doom_VERSION_MINOR 7)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Even though Clang and GCC seem to require specific flags for ISO C90,
# preserving this line seems the right thing to do as it may help with
# unlisted compilers in our conditional chain below.
set (C_STANDARD 90)

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
