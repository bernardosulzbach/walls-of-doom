cmake_minimum_required (VERSION 2.8.7)

configure_file (version.h.in version.h)

add_library (walls-of-doom-source
    about.h about.c
    box.h box.c
    command.h
    constants.h
    data.h data.c
    game.h game.c
    io.h io.c
    logger.h logger.c
    math.h math.c
    menu.h menu.c
    perks.h perks.c
    physics.h physics.c
    platform.h
    player.h player.c
    random.h random.c
    record.h record.c
    rest.h rest.c
    sort.h sort.c
    vector.h vector.c
    version.h)

set_target_properties (walls-of-doom-source PROPERTIES LINKER_LANGUAGE C)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})
target_link_libraries(walls-of-doom-source ${CURSES_LIBRARIES})

target_link_libraries (walls-of-doom-source m)

target_include_directories (walls-of-doom-source PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
