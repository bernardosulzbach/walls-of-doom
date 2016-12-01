#include "io.h"
#include "clock.h"
#include "constants.h"
#include "game.h"
#include "logger.h"
#include "memory.h"
#include "numeric.h"
#include "physics.h"
#include "player.h"
#include "profiler.h"
#include "random.h"
#include "record.h"
#include "settings.h"
#include "text.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define GAME_NAME "Walls of Doom"

#define CREATE_SURFACE_FAIL "Failed to create surface in %s!"
#define CREATE_TEXTURE_FAIL "Failed to create texture in %s!"

#define MINIMUM_BAR_HEIGHT 20

#define IMG_FLAGS IMG_INIT_PNG

static TTF_Font *global_monospaced_font = NULL;

/* Default integers to one to prevent divisions by zero. */
static int bar_height = 1;
static int window_width = 1;
static int window_height = 1;
static int global_monospaced_font_width = 1;
static int global_monospaced_font_height = 1;

/**
 * Clears the screen.
 */
void clear(SDL_Renderer *renderer) { SDL_RenderClear(renderer); }

/**
 * Updates the screen with what has been rendered.
 */
void present(SDL_Renderer *renderer) { SDL_RenderPresent(renderer); }

/**
 * In the future we may support window resizing.
 *
 * These functions encapsulate how window metrics are actually obtained.
 */
static int get_window_width(void) { return window_width; }

static int get_window_height(void) { return window_height; }

/**
 * Returns the height of the top and bottom bars.
 */
static int get_bar_height(void) { return bar_height; }

static int get_font_width(void) { return global_monospaced_font_width; }

static int get_font_height(void) { return global_monospaced_font_height; }

static int get_tile_width(void) {
  return get_window_width() / (get_columns() + 2);
}

static int get_tile_height(void) {
  return (get_window_height() - 2 * get_bar_height()) / (get_lines() + 2);
}

static int get_border_width(void) {
  const int center_width = get_window_width();
  return (center_width - get_columns() * get_tile_width()) / 2;
}

static int get_border_height(void) {
  const int center_height = get_window_height() - 2 * get_bar_height();
  return (center_height - get_lines() * get_tile_height()) / 2;
}

static int derive_font_size(void) { return (int)(get_bar_height() * 0.6); }

/**
 * Initializes the global fonts.
 */
static Code initialize_fonts(void) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  TTF_Font *font = NULL;
  if (global_monospaced_font != NULL) {
    return CODE_OK;
  }
  /* We try to open the font if we need to initialize. */
  font = TTF_OpenFont(MONOSPACED_FONT_PATH, derive_font_size());
  printf("%d\n", derive_font_size());
  /* If it failed, we log an error. */
  if (font == NULL) {
    sprintf(log_buffer, "TTF font opening error: %s", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  } else {
    global_monospaced_font = font;
  }
  return CODE_OK;
}

/**
 * Initializes the required font metrics.
 */
static Code initialize_font_metrics(void) {
  int width;
  int height;
  TTF_Font *font = global_monospaced_font;
  if (TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &width)) {
    log_message("Could not assess the width of a font");
    return CODE_ERROR;
  }
  height = TTF_FontHeight(font);
  global_monospaced_font_width = width;
  global_monospaced_font_height = height;
  return CODE_OK;
}

/**
 * Creates a new fullscreen window.
 *
 * Updates the pointers with the window width, window height, and bar height.
 */
static SDL_Window *create_window(int *width, int *height, int *bar_height) {
  const char *title = GAME_NAME;
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;
  Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  int line_height;
  /* Width and height do not matter because it is fullscreen. */
  SDL_Window *window = SDL_CreateWindow(title, x, y, 1, 1, flags);
  SDL_GetWindowSize(window, width, height);
  line_height = (*height - 2 * MINIMUM_BAR_HEIGHT) / get_lines();
  *bar_height = (*height - get_lines() * line_height) / 2;
  return window;
}

static Code set_window_title_and_icon(SDL_Window *window) {
  SDL_Surface *icon_surface = IMG_Load(ICON_PATH);
  SDL_SetWindowTitle(window, GAME_NAME);
  if (icon_surface == NULL) {
    log_message("Failed to load the window icon");
    return CODE_ERROR;
  }
  SDL_SetWindowIcon(window, icon_surface);
  SDL_FreeSurface(icon_surface);
  return CODE_OK;
}

static void set_render_color(SDL_Renderer *renderer, Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 */
Code initialize(SDL_Window **window, SDL_Renderer **renderer) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  Uint32 renderer_flags = 0;
  initialize_logger();
  initialize_profiler();
  initialize_settings();
  /* Initialize SDL. */
  if (SDL_Init(SDL_INIT_VIDEO)) {
    sprintf(log_buffer, "SDL initialization error: %s", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /* Initialize TTF. */
  if (!TTF_WasInit()) {
    if (TTF_Init()) {
      sprintf(log_buffer, "TTF initialization error: %s", SDL_GetError());
      log_message(log_buffer);
      return CODE_ERROR;
    }
  }
  if ((IMG_Init(IMG_FLAGS) & IMG_FLAGS) != IMG_FLAGS) {
    sprintf(log_buffer, "Failed to initialize required image support");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /**
   * The number of columns and the number of lines are fixed. However, the
   * number of pixels we need for the screen is not. We find this number by
   * experimenting before creating the window.
   */
  /* Log the size of the window we are going to create. */
  *window = create_window(&window_width, &window_height, &bar_height);
  sprintf(log_buffer, "Created a %dx%d window", window_width, window_height);
  log_message(log_buffer);
  if (*window == NULL) {
    sprintf(log_buffer, "SDL initialization error: %s", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  }
  if (initialize_fonts()) {
    sprintf(log_buffer, "Failed to initialize fonts");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  if (initialize_font_metrics()) {
    sprintf(log_buffer, "Failed to initialize font metrics");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /* Must disable text input to prevent a name capture bug. */
  SDL_StopTextInput();
  set_window_title_and_icon(*window);
  if (get_renderer_type() == RENDERER_HARDWARE) {
    renderer_flags = SDL_RENDERER_ACCELERATED;
  } else {
    renderer_flags = SDL_RENDERER_SOFTWARE;
  }
  *renderer = SDL_CreateRenderer(*window, -1, renderer_flags);
  set_render_color(*renderer, COLOR_DEFAULT_BACKGROUND);
  clear(*renderer);
  return CODE_OK;
}

/**
 * Finalizes the global fonts.
 */
static void finalize_fonts(void) {
  if (global_monospaced_font != NULL) {
    TTF_CloseFont(global_monospaced_font);
    global_monospaced_font = NULL;
  }
}

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 */
Code finalize(SDL_Window **window, SDL_Renderer **renderer) {
  finalize_fonts();
  SDL_DestroyRenderer(*renderer);
  SDL_DestroyWindow(*window);
  *window = NULL;
  if (TTF_WasInit()) {
    TTF_Quit();
  }
  /* This could be called earlier, but we only do it here to organize things. */
  IMG_Quit();
  SDL_Quit();
  finalize_profiler();
  finalize_logger();
  return CODE_OK;
}

/**
 * Evaluates whether or not a Player name is a valid name.
 *
 * A name is considered to be valid if it has at least two characters after
 * being trimmed.
 */
int is_valid_player_name(const char *player_name) {
  char buffer[MAXIMUM_PLAYER_NAME_SIZE];
  copy_string(buffer, player_name, MAXIMUM_PLAYER_NAME_SIZE);
  trim_string(buffer);
  return strlen(buffer) >= 2;
}

/**
 * Attempts to read a player name.
 *
 * Returns a Code, which may indicate that the player tried to quit.
 */
Code read_player_name(char *destination, const size_t maximum_size,
                      SDL_Renderer *renderer) {
  int x;
  int y;
  Code code = CODE_ERROR;
  int valid_name = 0;
  const char message[] = "Name your character: ";
  char log_buffer[MAXIMUM_STRING_SIZE];
  random_name(destination);
  /* While there is not a read error or a valid name. */
  while (code != CODE_OK || !valid_name) {
    x = get_padding() * get_font_width();
    y = (get_window_height() - get_font_height()) / 2;
    code = read_string(x, y, message, destination, maximum_size, renderer);
    if (code == CODE_QUIT) {
      return CODE_QUIT;
    } else if (code == CODE_OK) {
      sprintf(log_buffer, "Read '%s' from the user", destination);
      log_message(log_buffer);
      /* Trim the name the user entered. */
      trim_string(destination);
      sprintf(log_buffer, "Trimmed the input to '%s'", destination);
      log_message(log_buffer);
      valid_name = is_valid_player_name(destination);
    }
  }
  return code;
}

Code print_absolute(const int x, const int y, const char *string,
                    const ColorPair color_pair, SDL_Renderer *renderer) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  const SDL_Color foreground = to_sdl_color(color_pair.foreground);
  const SDL_Color background = to_sdl_color(color_pair.background);
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position;
  position.x = x;
  position.y = y;
  if (string == NULL || string[0] == '\0') {
    return CODE_OK;
  }
  /* Validate that x and y are nonnegative. */
  if (x < 0 || y < 0) {
    return CODE_ERROR;
  }
  surface = TTF_RenderText_Shaded(font, string, foreground, background);
  if (surface == NULL) {
    sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_absolute()");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_absolute()");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  return CODE_OK;
}

/**
 * Prints the provided string on the screen starting at (x, y).
 */
Code print(const int x, const int y, const char *string,
           const ColorPair color_pair, SDL_Renderer *renderer) {
  const int absolute_x = get_tile_width() * x;
  const int absolute_y = get_bar_height() + get_tile_height() * (y - 1);
  return print_absolute(absolute_x, absolute_y, string, color_pair, renderer);
}

static void swap_color(SDL_Renderer *renderer, SDL_Color *color) {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
}

static SDL_Rect make_top_bar_rectangle(void) {
  SDL_Rect rectangle;
  rectangle.x = 0;
  rectangle.y = get_bar_height();
  rectangle.w = get_window_width();
  rectangle.h = get_border_height();
  return rectangle;
}

static SDL_Rect make_left_bar_rectangle(void) {
  SDL_Rect rectangle;
  rectangle.x = 0;
  rectangle.y = get_bar_height();
  rectangle.w = get_border_width();
  rectangle.h = get_window_height() - 2 * get_bar_height();
  return rectangle;
}

static SDL_Rect make_right_bar_rectangle(void) {
  SDL_Rect rectangle;
  rectangle.x = get_window_width() - get_border_width();
  rectangle.y = get_bar_height();
  rectangle.w = get_border_width();
  rectangle.h = get_window_height() - 2 * get_bar_height();
  return rectangle;
}

static SDL_Rect make_bottom_bar_rectangle(void) {
  SDL_Rect rectangle;
  rectangle.x = 0;
  rectangle.y = get_window_height() - get_bar_height() - get_border_height();
  rectangle.w = get_window_width();
  rectangle.h = get_border_height();
  return rectangle;
}

static Code render_borders(SDL_Renderer *renderer) {
  const SDL_Rect top_bar = make_top_bar_rectangle();
  const SDL_Rect left_bar = make_left_bar_rectangle();
  const SDL_Rect right_bar = make_right_bar_rectangle();
  const SDL_Rect bottom_bar = make_bottom_bar_rectangle();
  SDL_Color color = to_sdl_color(COLOR_DEFAULT_FOREGROUND);
  swap_color(renderer, &color);
  SDL_RenderFillRect(renderer, &top_bar);
  SDL_RenderFillRect(renderer, &left_bar);
  SDL_RenderFillRect(renderer, &right_bar);
  SDL_RenderFillRect(renderer, &bottom_bar);
  swap_color(renderer, &color);
  return CODE_OK;
}

/**
 * Prints the provided strings centered at the specified absolute line.
 */
Code print_centered_horizontally(const int y, const int string_count,
                                 char const *const *strings,
                                 const ColorPair color_pair,
                                 SDL_Renderer *renderer) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  const SDL_Color foreground = to_sdl_color(color_pair.foreground);
  const SDL_Color background = to_sdl_color(color_pair.background);
  const int slice_size = get_window_width() / string_count;
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position;
  int i;
  position.x = 0;
  position.y = y;
  /* Validate that x and y are nonnegative. */
  if (y < 0) {
    return CODE_ERROR;
  }
  for (i = 0; i < string_count; i++) {
    if (*strings[i] == '\0') {
      continue;
    }
    surface = TTF_RenderText_Shaded(font, strings[i], foreground, background);
    if (surface == NULL) {
      sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_centered_horizontally()");
      log_message(log_buffer);
      return CODE_ERROR;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
      sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_centered_horizontally()");
      log_message(log_buffer);
      return CODE_ERROR;
    }
    /* Copy destination width and height from the texture. */
    SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
    position.x = i * slice_size + (slice_size - position.w) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &position);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
  }
  return CODE_OK;
}

/**
 * Prints the provided strings centered in the middle of the screen.
 */
Code print_centered_vertically(const int string_count,
                               char const *const *strings,
                               const ColorPair color_pair,
                               SDL_Renderer *renderer) {
  const int text_line_height = global_monospaced_font_height;
  const int padding = 2 * get_padding() * global_monospaced_font_height;
  const int available_window_height = get_window_height() - padding;
  const int text_lines_limit = available_window_height / text_line_height;
  int printed_count = string_count;
  int y;
  int i;
  if (string_count > text_lines_limit) {
    printed_count = text_lines_limit;
  }
  y = (get_window_height() - string_count * text_line_height) / 2;
  for (i = 0; i < printed_count; i++) {
    print_centered_horizontally(y, 1, strings + i, color_pair, renderer);
    y += text_line_height;
  }
  return CODE_OK;
}

/**
 * Replaces the first line break of any sequence of line breaks by a space.
 */
static void remove_first_breaks(char *string) {
  char c;
  size_t i;
  int preserve = 0;
  for (i = 0; string[i] != '\0'; i++) {
    c = string[i];
    if (c == '\n') {
      if (!preserve) {
        string[i] = ' ';
        /* Stop erasing newlines. */
        preserve = 1;
      }
    } else {
      /* Not a newline, we can erase again. */
      preserve = 0;
    }
  }
}

int count_lines(char *const buffer) {
  size_t counter = 0;
  size_t i = 0;
  while (buffer[i] != '\0') {
    if (buffer[i] == '\n') {
      counter++;
    }
    i++;
  }
  return counter;
}

char *copy_first_line(char *source, char *destination) {
  while (*source != '\0' && *source != '\n') {
    *destination++ = *source++;
  }
  *destination = '\0';
  if (*source == '\0') {
    return source;
  } else {
    return source + 1;
  }
}

void print_menu(const int line_count, char const *const *const lines,
                SDL_Renderer *renderer) {
  clear(renderer);
  print_centered_vertically(line_count, lines, COLOR_PAIR_DEFAULT, renderer);
  present(renderer);
}

/**
 * Prints the provided string after formatting it to increase readability.
 */
void print_long_text(char *string, SDL_Renderer *renderer) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  const int font_width = global_monospaced_font_width;
  const int width = get_window_width() - 2 * get_padding() * font_width;
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Color color = to_sdl_color(COLOR_DEFAULT_FOREGROUND);
  SDL_Rect position;
  position.x = get_padding() * font_width;
  position.y = get_padding() * font_width;
  remove_first_breaks(string);
  clear(renderer);
  /* Validate that the string is not empty and that x and y are nonnegative. */
  if (string == NULL || string[0] == '\0') {
    return;
  }
  surface = TTF_RenderText_Blended_Wrapped(font, string, color, width);
  if (surface == NULL) {
    sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_long_text()");
    log_message(log_buffer);
    return;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_long_text()");
    log_message(log_buffer);
    return;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, NULL, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  present(renderer);
}

/**
 * Draws an absolute rectangle based on the provided coordinates.
 */
static void draw_absolute_rectangle(const int x, const int y, const int w,
                                    const int h, Color color,
                                    SDL_Renderer *renderer) {
  SDL_Color swap = to_sdl_color(color);
  SDL_Rect rectangle;
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  swap_color(renderer, &swap);
  SDL_RenderFillRect(renderer, &rectangle);
  swap_color(renderer, &swap);
}

/**
 * Draws a relative rectangle based on the provided coordinates.
 */
static void draw_rectangle(int x, int y, int w, int h, Color color,
                           SDL_Renderer *renderer) {
  x = get_border_width() + x * get_tile_width();
  y = get_bar_height() + get_border_height() + y * get_tile_height();
  w = w * get_tile_width();
  h = h * get_tile_height();
  draw_absolute_rectangle(x, y, w, h, color, renderer);
}

static void write_top_bar_strings(const char *strings[],
                                  SDL_Renderer *renderer) {
  const ColorPair color_pair = COLOR_PAIR_TOP_BAR;
  const int string_count = TOP_BAR_STRING_COUNT;
  const int y = (get_bar_height() - global_monospaced_font_height) / 2;
  int h = get_bar_height();
  int w = get_window_width();
  draw_absolute_rectangle(0, 0, w, h, color_pair.background, renderer);
  print_centered_horizontally(y, string_count, strings, color_pair, renderer);
}

/**
 * Draws the top status bar on the screen for a given Player.
 */
static void draw_top_bar(const Player *const player, SDL_Renderer *renderer) {
  char lives_buffer[MAXIMUM_STRING_SIZE];
  char score_buffer[MAXIMUM_STRING_SIZE];
  const char *strings[TOP_BAR_STRING_COUNT];
  const char *perk_name = "No Power";
  if (player->perk != PERK_NONE) {
    perk_name = get_perk_name(player->perk);
  }
  sprintf(lives_buffer, "Lives: %d", player->lives);
  sprintf(score_buffer, "Score: %ld", player->score);
  strings[0] = GAME_NAME;
  strings[1] = perk_name;
  strings[2] = lives_buffer;
  strings[3] = score_buffer;
  write_top_bar_strings((const char **)strings, renderer);
}

static void write_bottom_bar_string(const char *string,
                                    SDL_Renderer *renderer) {
  /* Use half a character for horizontal padding. */
  const int x = global_monospaced_font_width / 2;
  const int bar_start = get_window_height() - get_bar_height();
  const int padding = (get_bar_height() - global_monospaced_font_height) / 2;
  const int y = bar_start + padding;
  print_absolute(x, y, string, COLOR_PAIR_BOTTOM_BAR, renderer);
}

/*
 * Draws the bottom status bar on the screen for a given Player.
 */
static void draw_bottom_bar(const char *message, SDL_Renderer *renderer) {
  const Color color = COLOR_PAIR_BOTTOM_BAR.background;
  const int y = get_window_height() - bar_height;
  const int w = get_window_width();
  const int h = get_bar_height();
  draw_absolute_rectangle(0, y, w, h, color, renderer);
  write_bottom_bar_string(message, renderer);
}

/**
 * Draws the borders of the screen.
 */
static void draw_borders(SDL_Renderer *renderer) { render_borders(renderer); }

static void draw_platforms(const Platform *platforms,
                           const size_t platform_count, const BoundingBox *box,
                           SDL_Renderer *renderer) {
  Platform p;
  int x;
  int y;
  int w;
  size_t i;
  for (i = 0; i < platform_count; i++) {
    p = platforms[i];
    x = max_int(box->min_x, p.x);
    y = p.y;
    w = min_int(box->max_x, p.x + p.width - 1) - x + 1;
    draw_rectangle(x, y, w, 1, COLOR_PAIR_PLATFORM.foreground, renderer);
  }
}

static int has_active_perk(const Game *const game) {
  return game->perk != PERK_NONE;
}

static void draw_perk(const Game *const game, SDL_Renderer *renderer) {
  const Color color = COLOR_PAIR_PERK.background;
  if (has_active_perk(game)) {
    draw_rectangle(game->perk_x, game->perk_y, 1, 1, color, renderer);
  }
}

Code draw_player(const Player *const player, SDL_Renderer *renderer) {
  const int x = player->x;
  const int y = player->y;
  draw_rectangle(x, y, 1, 1, COLOR_PAIR_PLAYER.foreground, renderer);
  return CODE_OK;
}

/**
 * Draws a full game to the screen.
 *
 * Returns a Milliseconds approximation of the time this function took.
 */
Milliseconds draw_game(const Game *const game, SDL_Renderer *renderer) {
  Milliseconds draw_game_start = get_milliseconds();

  profiler_begin("draw_game:clear");
  clear(renderer);
  profiler_end("draw_game:clear");

  profiler_begin("draw_game:draw_top_bar");
  draw_top_bar(game->player, renderer);
  profiler_end("draw_game:draw_top_bar");

  profiler_begin("draw_game:draw_bottom_bar");
  draw_bottom_bar(game->message, renderer);
  profiler_end("draw_game:draw_bottom_bar");

  profiler_begin("draw_game:draw_borders");
  draw_borders(renderer);
  profiler_end("draw_game:draw_borders");

  profiler_begin("draw_game:draw_platforms");
  draw_platforms(game->platforms, game->platform_count, game->box, renderer);
  profiler_end("draw_game:draw_platforms");

  profiler_begin("draw_game:draw_perk");
  draw_perk(game, renderer);
  profiler_end("draw_game:draw_perk");

  profiler_begin("draw_game:draw_player");
  draw_player(game->player, renderer);
  profiler_end("draw_game:draw_player");

  profiler_begin("draw_game:present");
  present(renderer);
  profiler_end("draw_game:present");

  update_profiler("draw_game", get_milliseconds() - draw_game_start);
  return get_milliseconds() - draw_game_start;
}

void print_game_result(const char *name, const unsigned int score,
                       const int position, SDL_Renderer *renderer) {
  const ColorPair color = COLOR_PAIR_DEFAULT;
  char first_line[MAXIMUM_STRING_SIZE];
  char second_line[MAXIMUM_STRING_SIZE];
  char *lines[3];
  sprintf(first_line, "%s died after making %d points.", name, score);
  if (position > 0) {
    sprintf(second_line, "%s got to position %d!", name, position);
  } else {
    sprintf(second_line, "%s didn't make it to the top scores.", name);
  }
  clear(renderer);
  lines[0] = first_line;
  lines[1] = "";
  lines[2] = second_line;
  print_centered_vertically(3, (const char *const *)lines, color, renderer);
  present(renderer);
}

/**
 * Converts a Record to a human-readable string.
 */
static void record_to_string(const Record *const record, char *dest,
                             const int width) {
  const char format[] = "%s%*.*s%d";
  const char *name = record->name;
  const int score = record->score;
  char pad_string[MAXIMUM_STRING_SIZE];
  int pad_length;
  memset(pad_string, '.', MAXIMUM_STRING_SIZE - 1);
  pad_string[MAXIMUM_STRING_SIZE - 1] = '\0';
  pad_length = width - strlen(name) - count_digits(score);
  sprintf(dest, format, name, pad_length, pad_length, pad_string, score);
}

void print_records(const size_t count, const Record *records,
                   SDL_Renderer *renderer) {
  const ColorPair pair = COLOR_PAIR_DEFAULT;
  const int x_padding = 2 * get_padding() * get_font_width();
  const int y_padding = 2 * get_padding() * get_font_height();
  const int available_window_height = get_window_height() - y_padding;
  const int text_lines_limit = available_window_height / get_font_height();
  const int text_width_in_pixels = get_window_width() - x_padding;
  const size_t string_width = text_width_in_pixels / get_font_width();
  const size_t printed = min_int(count, text_lines_limit);
  char **strings = NULL;
  size_t i;
  strings = resize_memory(strings, sizeof(char *) * printed);
  for (i = 0; i < printed; i++) {
    strings[i] = NULL;
    strings[i] = resize_memory(strings[i], string_width);
    record_to_string(records + i, strings[i], string_width - 1);
  }
  clear(renderer);
  print_centered_vertically(printed, (const char *const *)strings, pair,
                            renderer);
  present(renderer);
  for (i = 0; i < printed; i++) {
    resize_memory(strings[i], 0);
  }
  resize_memory(strings, 0);
}

/**
 * Asserts whether or not a character is a valid input character.
 *
 * For simplicity, the user should only be able to enter letters and numbers.
 */
int is_valid_input_character(char c) { return isalnum(c); }

/**
 * Prints a string starting from (x, y) but limits to its first limit
 * characters.
 */
static void print_limited(const int x, const int y, const char *string,
                          const size_t limit, SDL_Renderer *renderer) {
  const char ellipsis[] = "...";
  const size_t string_length = strlen(string);
  const size_t ellipsis_length = strlen(ellipsis);
  const size_t minimum_string_size_for_ellipsis = 2 * ellipsis_length;
  /* No-op. */
  if (limit < 1) {
    return;
  }
  /*
   * As only two calls to print suffice to solve this problem for any possible
   * input size, we avoid using a dynamically allocated buffer to prepare the
   * output.
   */
  /* String length is less than the limit. */
  if (string_length < limit) {
    print_absolute(x, y, string, COLOR_PAIR_DEFAULT, renderer);
    return;
  }
  /* String is longer than the limit. */
  /* Write the ellipsis if we need to. */
  if (limit >= minimum_string_size_for_ellipsis) {
    print_absolute(x, y, ellipsis, COLOR_PAIR_DEFAULT, renderer);
  }
  /* Write the tail of the input string. */
  string += string_length - limit + ellipsis_length;
  print_absolute(x + ellipsis_length, y, string, COLOR_PAIR_DEFAULT, renderer);
}

/**
 * Reads a string from the user of up to size characters (including NUL).
 */
Code read_string(const int x, const int y, const char *prompt,
                 char *destination, const size_t size, SDL_Renderer *renderer) {
  const int buffer_x = x + (strlen(prompt) + 1) * get_font_width();
  const int padding_size = get_padding() * get_font_width();
  const int buffer_view_size = get_window_width() - buffer_x - padding_size;
  const int buffer_view_limit = buffer_view_size / get_font_width();
  int is_done = 0;
  int should_rerender = 1;
  /* The x coordinate of the user input buffer. */
  size_t written = strlen(destination);
  char character = '\0';
  char *write = destination + written;
  SDL_Event event;
  /* Start listening for text input. */
  SDL_StartTextInput();
  while (!is_done) {
    if (should_rerender) {
      clear(renderer);
      print_absolute(x, y, prompt, COLOR_PAIR_DEFAULT, renderer);
      if (written == 0) {
        /* We must write a single space, or SDL will not render anything. */
        print_absolute(buffer_x, y, " ", COLOR_PAIR_DEFAULT, renderer);
      } else {
        /*
         * Must care about how much we write, padding should be respected.
         *
         * Simply enough, we print the tail of the string and omit the
         * beginning with an ellipsis.
         *
         */
        print_limited(buffer_x, y, destination, buffer_view_limit, renderer);
      }
      present(renderer);
      should_rerender = 0;
    }
    /* Throughout the loop, the write pointer always points to a '\0'. */
    if (SDL_WaitEvent(&event)) {
      /* Check for user quit and return 1. */
      /* This is OK because the destination string is always a valid C string.
       */
      if (event.type == SDL_QUIT) {
        return CODE_QUIT;
      } else if (event.type == SDL_KEYDOWN) {
        /* Handle backspace. */
        if (event.key.keysym.sym == SDLK_BACKSPACE && written > 0) {
          write--;
          *write = '\0';
          written--;
          should_rerender = 1;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
          is_done = 1;
        }
      } else if (event.type == SDL_TEXTINPUT) {
        character = event.text.text[0];
        if (is_valid_input_character(character) && written + 1 < size) {
          *write = character;
          write++;
          written++;
          /* Terminate the string with NIL to ensure it is valid. */
          *write = '\0';
          should_rerender = 1;
        }
        /* We could handle copying and pasting by checking for Control-C. */
        /* SDL_*ClipboardText() allows us to access the clipboard text. */
      }
    }
  }
  /* Stop listening for text input. */
  SDL_StopTextInput();
  return CODE_OK;
}
