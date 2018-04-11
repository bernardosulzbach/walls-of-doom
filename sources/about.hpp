#ifndef ABOUT_H
#define ABOUT_H

#include "code.hpp"
#include "command.hpp"
#include "io.hpp"
#include "settings.hpp"
#include <SDL.h>

/**
 * Presents information about the game to the player.
 */
Code info(const Settings &settings, Renderer &renderer, CommandTable *table);

#endif
