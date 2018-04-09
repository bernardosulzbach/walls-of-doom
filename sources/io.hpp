#ifndef BASE_IO_H
#define BASE_IO_H

#include "clock.hpp"
#include "code.hpp"
#include "color.hpp"
#include "command.hpp"
#include "game.hpp"
#include "perk.hpp"
#include "physics.hpp"
#include "record.hpp"
#include <SDL_ttf.h>
#include <string>
#include <vector>

typedef TTF_Font Font;
typedef SDL_Window Window;
typedef SDL_Renderer Renderer;

extern const char *game_name;

/**
 * Clears the screen.
 */
void clear(Renderer *renderer);

/**
 * Updates the screen with what has been rendered.
 */
void present(Renderer *renderer);

Font *get_font();

int get_font_width();

int get_font_height();

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 */
Code initialize(Settings &settings, Window **window, Renderer **renderer);

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 */
Code finalize(Window **window, Renderer **renderer);

Code print_absolute(int x, int y, const char *string, ColorPair color, Renderer *renderer);

Code print_centered_vertically(const Settings &settings, const std::vector<std::string> &strings, ColorPair color,
                               Renderer *renderer);

/**
 * Reads a string from the user of up to size characters (including NUL).
 *
 * The string will be echoed after the prompt, which starts at (x, y).
 */
Code read_string(const Settings &settings, int x, int y, const char *prompt, char *destination, size_t size,
                 Renderer *renderer);

/**
 * Attempts to read a player name.
 *
 * Returns a Code, which may indicate that the player tried to quit.
 */
Code read_player_name(const Context &context, std::string &destination, Renderer *renderer);

/**
 * Draws a full game to the screen.
 *
 * Returns a Milliseconds approximation of the time this function took.
 */
Milliseconds draw_game(Game *game, Renderer *renderer);

Code top_scores(const Settings &settings, Profiler &profiler, Renderer *renderer, CommandTable *table);

void print_menu(const Settings &settings, const std::vector<std::string> &lines, Renderer *renderer);

/**
 * Prints the provided string after formatting it to increase readability.
 */
void print_long_text(const Settings &settings, char *string, Renderer *renderer);

#endif
