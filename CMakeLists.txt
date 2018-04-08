cmake_minimum_required(VERSION 2.8.7)

project(walls-of-doom LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Get Git version, if we can.
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(COMMAND git describe --abbrev=4 HEAD OUTPUT_VARIABLE version-string OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
    set(version-string "v1.4.0")
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(ENV32 "Generate code for a 32-bit environment.")
option(ENV64 "Generate code for a 64-bit environment.")
option(SANITIZE "Modify the program at compile-time to catch undefined behavior during program execution.")
option(OPTIMIZE_SIZE "Optimize for program size.")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (ENV32)
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    elseif (ENV64)
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    endif ()
    if (OPTIMIZE_SIZE)
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    endif ()
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (SANITIZE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    endif ()
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Run-time buffer overflow detection.
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        add_definitions(-D_FORTIFY_SOURCE=2)
    endif ()
    # Run-time bounds checking for C++ strings and containers.
    add_definitions(-D_GLIBCXX_ASSERTIONS)
    add_definitions(-fasynchronous-unwind-tables)
    add_definitions(-fexceptions)
    add_definitions(-fpie)
endif ()

configure_file(sources/version.hpp.in version.hpp)
configure_file(sources/constants.hpp.in constants.hpp)

set(walls-of-doom-sources
        sources/about.hpp
        sources/about.cpp
        sources/analyst.hpp
        sources/analyst.cpp
        sources/box.hpp
        sources/box.cpp
        sources/clock.hpp
        sources/clock.cpp
        sources/code.hpp
        sources/color.hpp
        sources/color.cpp
        sources/command.hpp
        sources/command.cpp
        sources/constants.hpp
        sources/data.hpp
        sources/data.cpp
        sources/integers.hpp
        sources/game.hpp
        sources/game.cpp
        sources/graphics.hpp
        sources/io.hpp
        sources/io.cpp
        sources/joystick.hpp
        sources/joystick.cpp
        sources/logger.hpp
        sources/logger.cpp
        sources/menu.hpp
        sources/menu.cpp
        sources/numeric.hpp
        sources/numeric.cpp
        sources/perk.hpp
        sources/perk.cpp
        sources/physics.hpp
        sources/physics.cpp
        sources/platform.hpp
        sources/platform.cpp
        sources/player.hpp
        sources/player.cpp
        sources/point.hpp
        sources/profiler.hpp
        sources/profiler.cpp
        sources/random.hpp
        sources/random.cpp
        sources/record.hpp
        sources/record.cpp
        sources/score.hpp
        sources/settings.hpp
        sources/settings.cpp
        sources/sort.hpp
        sources/sort.cpp
        sources/text.hpp
        sources/text.cpp
        sources/version.hpp
        sources/record_table.cpp
        sources/record_table.hpp)

add_library(walls-of-doom-object OBJECT ${walls-of-doom-sources})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang|Intel")
    add_definitions(-Wall)
    add_definitions(-Wextra)
    add_definitions(-Wpedantic)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
        add_definitions(-Wunused)
        add_definitions(-Wcast-align)
        add_definitions(-Wold-style-cast)
        add_definitions(-Woverloaded-virtual)
        add_definitions(-Wmissing-field-initializers)
    endif ()
    add_definitions(-Werror)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    add_definitions(/W4)
endif ()

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(walls-of-doom sources/main.cpp $<TARGET_OBJECTS:walls-of-doom-object>)

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(walls-of-doom ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

add_custom_command(TARGET walls-of-doom POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ ${CMAKE_CURRENT_BINARY_DIR}/assets/)

if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    add_executable(tests tests/tests.cpp $<TARGET_OBJECTS:walls-of-doom-object>)
    include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/catch")
    include_directories("${CMAKE_SOURCE_DIR}/sources")
    target_link_libraries(tests ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif ()
