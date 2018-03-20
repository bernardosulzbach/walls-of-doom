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
 * Swap the renderer color by the provided color.
 *
 * The color previously in the renderer will be copied to the pointer.
 */
void swap_color(Renderer *renderer, SDL_Color *color);

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 */
Code initialize(Window **window, Renderer **renderer);

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 */
Code finalize(Window **window, Renderer **renderer);

Code print_absolute(int x, int y, const char *string, ColorPair color_pair, Renderer *renderer);

/**
 * Prints the provided strings centered at the specified absolute line.
 */
Code print_centered_horizontally(int y, const std::vector<std::string> &strings, ColorPair pair, Renderer *renderer);

Code print_centered_vertically(const std::vector<std::string> &strings, ColorPair pair, Renderer *renderer);

/**
 * Reads a string from the user of up to size characters (including NUL).
 *
 * The string will be echoed after the prompt, which starts at (x, y).
 */
Code read_string(int x, int y, const char *prompt, char *destination, size_t size, Renderer *renderer);

/**
 * Attempts to read a player name.
 *
 * Returns a Code, which may indicate that the player tried to quit.
 */
Code read_player_name(char *destination, size_t maximum_size, Renderer *renderer);

/**
 * Draws a full game to the screen.
 *
 * Returns a Milliseconds approximation of the time this function took.
 */
Milliseconds draw_game(Game *const game, Renderer *renderer);

void print_menu(const std::vector<std::string> &lines, Renderer *renderer);

/**
 * Prints the provided string after formatting it to increase readability.
 */
void print_long_text(char *string, Renderer *renderer);

void print_records(size_t count, const Record *records, Renderer *renderer);

#endif