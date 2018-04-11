#include "io.hpp"
#include "clock.hpp"
#include "constants.hpp"
#include "game.hpp"
#include "joystick.hpp"
#include "logger.hpp"
#include "numeric.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "profiler.hpp"
#include "random.hpp"
#include "record.hpp"
#include "record_table.hpp"
#include "settings.hpp"
#include "text.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cctype>
#include <cstdio>
#include <cstring>

#define CREATE_SURFACE_FAIL "Failed to create surface in %s!"
#define CREATE_TEXTURE_FAIL "Failed to create texture in %s!"

#define IMG_FLAGS IMG_INIT_PNG

#define SDL_INIT_FLAGS (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)

const char *const game_name = "Walls of Doom";

static TTF_Font *global_monospaced_font = nullptr;

/* Default integers to one to prevent divisions by zero. */
static int window_width = 1;
static int window_height = 1;
static int global_monospaced_font_width = 1;
static int global_monospaced_font_height = 1;

static const int PERK_FADING_INTERVAL = UPS;

Renderer::Renderer(Context &context) : context(context) {
  initialize(context.settings);
}

Renderer::~Renderer() {
  finalize();
}

/**
 * Initializes the required resources.
 *
 * Should only be called once, right after starting.
 */
Code Renderer::initialize(Settings &settings) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  U32 renderer_flags = 0;
  initialize_logger();
  if (SDL_Init(SDL_INIT_FLAGS) != 0) {
    sprintf(log_buffer, "SDL initialization error: %s.", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  }
  SDL_DisplayMode display_mode{};
  SDL_GetCurrentDisplayMode(0, &display_mode);
  settings.compute_window_size(static_cast<U32>(display_mode.w), static_cast<U32>(display_mode.h));
  settings.validate_settings();
  SDL_ShowCursor(SDL_DISABLE);
  initialize_joystick();
  if (TTF_WasInit() == 0) {
    if (TTF_Init() != 0) {
      sprintf(log_buffer, "TTF initialization error: %s.", SDL_GetError());
      log_message(log_buffer);
      return CODE_ERROR;
    }
  }
  if ((IMG_Init(IMG_FLAGS) & IMG_FLAGS) != IMG_FLAGS) {
    sprintf(log_buffer, "Failed to initialize required image support.");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  create_window(settings, &window_width, &window_height);
  // Log the size of the window we are going to create.
  sprintf(log_buffer, "Created a %dx%d window.", window_width, window_height);
  log_message(log_buffer);
  if (window == nullptr) {
    sprintf(log_buffer, "SDL initialization error: %s.", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  }
  if (initialize_fonts(settings) != 0u) {
    sprintf(log_buffer, "Failed to initialize fonts.");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  if (initialize_font_metrics() != 0u) {
    sprintf(log_buffer, "Failed to initialize font metrics.");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /* Must disable text input to prevent a name capture bug. */
  SDL_StopTextInput();
  set_window_title_and_icon();
  if (settings.get_renderer_type() == RENDERER_HARDWARE) {
    renderer_flags = SDL_RENDERER_ACCELERATED;
  } else {
    renderer_flags = SDL_RENDERER_SOFTWARE;
  }
  renderer = SDL_CreateRenderer(window, -1, renderer_flags);
  set_color(COLOR_DEFAULT_BACKGROUND);
  clear();
  return CODE_OK;
}

/**
 * Initializes the global fonts.
 */
Code Renderer::initialize_fonts(const Settings &settings) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  TTF_Font *font = nullptr;
  if (global_monospaced_font != nullptr) {
    return CODE_OK;
  }
  /* We try to open the font if we need to initialize. */
  font = TTF_OpenFont(MONOSPACED_FONT_PATH, settings.get_font_size());
  /* If it failed, we log an error. */
  if (font == nullptr) {
    sprintf(log_buffer, "TTF font opening error: %s.", SDL_GetError());
    log_message(log_buffer);
    return CODE_ERROR;
  }
  global_monospaced_font = font;

  return CODE_OK;
}

/**
 * Initializes the required font metrics.
 */
Code Renderer::initialize_font_metrics() {
  int width;
  int height;
  TTF_Font *font = global_monospaced_font;
  if (TTF_GlyphMetrics(font, 'A', nullptr, nullptr, nullptr, nullptr, &width) != 0) {
    log_message("Could not assess the width of a font.");
    return CODE_ERROR;
  }
  height = TTF_FontHeight(font);
  global_monospaced_font_width = width;
  global_monospaced_font_height = height;
  return CODE_OK;
}

void Renderer::create_window(const Settings &settings, int *width, int *height) {
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;
  const int w = settings.get_window_width();
  const int h = settings.get_window_height();
  const Uint32 flags = SDL_WINDOW_INPUT_FOCUS;
  window = SDL_CreateWindow(game_name, x, y, w, h, flags);
  SDL_GetWindowSize(window, width, height);
}

Code Renderer::set_window_title_and_icon() {
  SDL_Surface *icon_surface = IMG_Load(ICON_PATH);
  SDL_SetWindowTitle(window, game_name);
  if (icon_surface == nullptr) {
    log_message("Failed to load the icon.");
    return CODE_ERROR;
  }
  SDL_SetWindowIcon(window, icon_surface);
  SDL_FreeSurface(icon_surface);
  return CODE_OK;
}

/**
 * Finalizes the acquired resources.
 *
 * Should only be called once, right before exiting.
 */
Code Renderer::finalize() {
  finalize_fonts();
  finalize_joystick();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  window = nullptr;
  if (TTF_WasInit() != 0) {
    TTF_Quit();
  }
  /* This could be called earlier, but we only do it here to organize things. */
  IMG_Quit();
  SDL_Quit();
  finalize_logger();
  return CODE_OK;
}

/**
 * Finalizes the global fonts.
 */
void Renderer::finalize_fonts() {
  if (global_monospaced_font != nullptr) {
    TTF_CloseFont(global_monospaced_font);
    global_monospaced_font = nullptr;
  }
}

/**
 * Clears the screen.
 */
void Renderer::clear() {
  set_color(COLOR_DEFAULT_BACKGROUND);
  SDL_RenderClear(renderer);
}

/**
 * Updates the screen with what has been rendered.
 */
void Renderer::present() {
  SDL_RenderPresent(renderer);
}

TTF_Font *Renderer::get_font() {
  return global_monospaced_font;
}

int Renderer::get_font_width() {
  return global_monospaced_font_width;
}

int Renderer::get_font_height() {
  return global_monospaced_font_height;
}

void Renderer::set_color(Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

Code Renderer::print_absolute(const int x, const int y, const std::string &string, const ColorPair color) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position{};
  position.x = x;
  position.y = y;
  if (string[0] == '\0') {
    return CODE_OK;
  }
  /* Validate that x and y are nonnegative. */
  if (x < 0 || y < 0) {
    return CODE_ERROR;
  }
  auto foreground_color = color.foreground.to_SDL_color();
  auto background_color = color.background.to_SDL_color();
  surface = TTF_RenderText_Shaded(font, string.c_str(), foreground_color, background_color);
  if (surface == nullptr) {
    sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_absolute()");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == nullptr) {
    sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_absolute()");
    log_message(log_buffer);
    return CODE_ERROR;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, nullptr, nullptr, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, nullptr, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  return CODE_OK;
}

void Renderer::remove_first_breaks(std::string &string) {
  int preserve = 0;
  for (size_t i = 0; string[i] != '\0'; i++) {
    const auto c = string[i];
    if (c == '\n') {
      if (preserve == 0) {
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

/**
 * Prints the provided string after formatting it to increase readability.
 */
void Renderer::print_long_text(std::string string) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  const int font_width = get_font_width();
  const int width = context.settings.get_window_width() - 2 * context.settings.get_padding() * font_width;
  TTF_Font *font = get_font();
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position{};
  position.x = context.settings.get_padding() * font_width;
  position.y = context.settings.get_padding() * font_width;
  clear();
  /* Validate that the string is not empty and that x and y are nonnegative. */
  if (string[0] == '\0') {
    return;
  }
  remove_first_breaks(string);
  surface = TTF_RenderText_Blended_Wrapped(font, string.c_str(), COLOR_DEFAULT_FOREGROUND.to_SDL_color(), width);
  if (surface == nullptr) {
    sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_long_text()");
    log_message(log_buffer);
    return;
  }
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == nullptr) {
    sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_long_text()");
    log_message(log_buffer);
    return;
  }
  /* Copy destination width and height from the texture. */
  SDL_QueryTexture(texture, nullptr, nullptr, &position.w, &position.h);
  SDL_RenderCopy(renderer, texture, nullptr, &position);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  present();
}

/**
 * Prints the provided strings centered at the specified absolute line.
 */
Code Renderer::print_centered_horizontally(int y, const std::vector<std::string> &strings, ColorPair color) {
  char log_buffer[MAXIMUM_STRING_SIZE];
  const SDL_Color foreground = color.foreground.to_SDL_color();
  const SDL_Color background = color.background.to_SDL_color();
  const auto slice_size = context.settings.get_window_width() / strings.size();
  TTF_Font *font = global_monospaced_font;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect position{};
  position.x = 0;
  position.y = y;
  /* Validate that x and y are nonnegative. */
  if (y < 0) {
    return CODE_ERROR;
  }
  for (int i = 0; i < static_cast<int>(strings.size()); i++) {
    if (strings[i].empty()) {
      continue;
    }
    surface = TTF_RenderText_Shaded(font, strings[i].c_str(), foreground, background);
    if (surface == nullptr) {
      sprintf(log_buffer, CREATE_SURFACE_FAIL, "print_centered_horizontally()");
      log_message(log_buffer);
      return CODE_ERROR;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
      sprintf(log_buffer, CREATE_TEXTURE_FAIL, "print_centered_horizontally()");
      log_message(log_buffer);
      return CODE_ERROR;
    }
    /* Copy destination width and height from the texture. */
    SDL_QueryTexture(texture, nullptr, nullptr, &position.w, &position.h);
    position.x = i * slice_size + (slice_size - position.w) / 2;
    SDL_RenderCopy(renderer, texture, nullptr, &position);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
  }
  return CODE_OK;
}

/**
 * Prints the provided strings centered in the middle of the screen.
 */
Code Renderer::print_centered_vertically(const std::vector<std::string> &strings, ColorPair color) {
  const int text_line_height = global_monospaced_font_height;
  const int padding = 2 * context.settings.get_padding() * global_monospaced_font_height;
  const int available_window_height = context.settings.get_window_height() - padding;
  const int text_lines_limit = available_window_height / text_line_height;
  const auto printed_count = std::min(text_lines_limit, static_cast<int>(strings.size()));
  auto y = (context.settings.get_window_height() - strings.size() * text_line_height) / 2;
  for (int i = 0; i < printed_count; i++) {
    const auto string = strings[i];
    auto *array = reinterpret_cast<char *>(malloc(string.size() + 1));
    copy_string(array, string.c_str(), string.size() + 1);
    print_centered_horizontally(y, {string}, color);
    y += text_line_height;
    free(array);
  }
  return CODE_OK;
}

bool Renderer::is_valid_player_name(const std::string &player_name) {
  return player_name.size() >= 2;
}

/**
 * Attempts to read a player name.
 *
 * Returns a Code, which may indicate that the player tried to quit.
 */
Code Renderer::read_player_name(std::string &destination) {
  Code code = CODE_ERROR;
  bool valid_name = false;
  const char message[] = "Name your character: ";
  destination = context.random_number_generator.get_user_name();
  /* While there is not a read error or a valid name. */
  std::vector<char> name(maximum_player_name_size);
  copy_string(name.data(), destination.c_str(), maximum_player_name_size);
  while (code != CODE_OK || !valid_name) {
    auto x = context.settings.get_padding() * get_font_width();
    auto y = (context.settings.get_window_height() - get_font_height()) / 2;
    code = read_string(x, y, message, name.data(), maximum_player_name_size);
    if (code == CODE_QUIT) {
      return CODE_QUIT;
    }
    if (code == CODE_OK) {
      log_message("Read " + std::string(name.data()) + " from the user.");
      /* Trim the name the user entered. */
      trim_string(name.data());
      log_message("Trimmed the input to " + std::string(name.data()) + ".");
      valid_name = is_valid_player_name(std::string(name.data()));
    }
  }
  destination = std::string(name.data());
  return code;
}

void Renderer::print_menu(const std::vector<std::string> &lines) {
  clear();
  print_centered_vertically(lines, COLOR_PAIR_DEFAULT);
  present();
}

/**
 * Draws an absolute rectangle based on the provided coordinates.
 */
void Renderer::draw_absolute_rectangle(int x, int y, int w, int h, Color color) {
  SDL_Rect rectangle{};
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  set_color(color);
  SDL_RenderFillRect(renderer, &rectangle);
}

/**
 * Draws a shaded absolute rectangle based on the provided coordinates.
 */
void Renderer::draw_shaded_absolute_rectangle(int x, int y, int w, int h, Color color) {
  SDL_Rect rectangle{};
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  set_color(color);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(renderer, &rectangle);
}

void Renderer::draw_absolute_tile_rectangle(int x, int y, Color color) {
  const int w = context.settings.get_tile_w();
  const int h = context.settings.get_tile_h();
  y += context.settings.get_bar_height();
  draw_absolute_rectangle(x, y, w, h, color);
}

void Renderer::draw_shaded_absolute_tile_rectangle(int x, int y, Color color) {
  const int w = context.settings.get_tile_w();
  const int h = context.settings.get_tile_h();
  y += context.settings.get_bar_height();
  draw_shaded_absolute_rectangle(x, y, w, h, color);
}

void Renderer::write_top_bar_strings(const std::vector<std::string> &strings) {
  const auto color_pair = COLOR_PAIR_TOP_BAR;
  const auto y = (context.settings.get_bar_height() - get_font_height()) / 2;
  const auto h = context.settings.get_bar_height();
  const auto w = context.settings.get_window_width();
  draw_absolute_rectangle(0, 0, w, h, color_pair.background);
  print_centered_horizontally(y, strings, color_pair);
}

/**
 * Draws the top status bar on the screen for a given Player.
 */
void Renderer::draw_top_bar(const Game &game) {
  const Player *player = game.player;
  std::vector<std::string> strings;
  std::string perk_name = "No Power";
  if (player->perk != PERK_NONE) {
    perk_name = get_perk_name(player->perk);
  }
  const auto limit = game.limit_played_frames;
  const auto time_left = (limit - game.played_frames) / static_cast<double>(UPS);
  strings.push_back(double_to_string(time_left, 2));
  strings.push_back(perk_name);
  strings.push_back("Lives: " + std::to_string(player->lives));
  strings.push_back("Score: " + std::to_string(player->score));
  write_top_bar_strings(strings);
}

void Renderer::write_bottom_bar_string(const std::string &string) {
  /* Use half a character for horizontal padding. */
  const int x = get_font_width() / 2;
  const int bar_start = context.settings.get_window_height() - context.settings.get_bar_height();
  const int padding = (context.settings.get_bar_height() - get_font_height()) / 2;
  const int y = bar_start + padding;
  print_absolute(x, y, string, COLOR_PAIR_BOTTOM_BAR);
}

/*
 * Draws the bottom status bar on the screen for a given Player.
 */
void Renderer::draw_bottom_bar(const std::string &message) {
  const Color color = COLOR_PAIR_BOTTOM_BAR.background;
  const int y = context.settings.get_window_height() - context.settings.get_bar_height();
  const int w = context.settings.get_window_width();
  const int h = context.settings.get_bar_height();
  draw_absolute_rectangle(0, y, w, h, color);
  write_bottom_bar_string(message);
}

Color Renderer::get_platform_color(Platform platform) {
  return COLOR_PAIR_PLATFORM_A.foreground.mix(COLOR_PAIR_PLATFORM_B.foreground, platform.rarity);
}

void Renderer::draw_platforms(const std::vector<Platform> &platforms, BoundingBox box) {
  const auto y_padding = context.settings.get_bar_height();
  for (const auto &platform : platforms) {
    auto p = platform;
    auto x = std::max(box.min_x, p.x);
    auto y = y_padding + p.y;
    auto w = std::min(box.max_x, p.x + p.w - 1) - x + 1;
    auto h = p.h;
    draw_absolute_rectangle(x, y, w, h, get_platform_color(platform));
  }
}

bool Renderer::has_active_perk(const Game &game) {
  return game.perk != PERK_NONE;
}

void Renderer::draw_resized_perk(int x, int y, int w, int h, double f) {
  /* The scaled values. */
  const auto s_w = static_cast<int>(f * w);
  const auto s_h = static_cast<int>(f * h);
  /* The foreground variables. */
  const Color f_color = COLOR_PAIR_PERK.background;
  int f_x;
  int f_y;
  int f_w = s_w;
  int f_h = s_h;
  /* The background variables. */
  const Color b_color = COLOR_PAIR_DEFAULT.background.mix(f_color, 0.5f);
  int b_x;
  int b_y;
  int b_w = s_w;
  int b_h = s_h;
  /* If the width or height is reduced by 2, reduce back and front. */
  /* Otherwise, reduce inner on each side and interpolate back. */
  if ((w - s_w) % 2 == 1) {
    b_w = s_w + 1;
    f_w = s_w - 1;
  }
  if ((h - s_h) % 2 == 1) {
    b_h = s_h + 1;
    f_h = s_h - 1;
  }
  f_x = x + (w - f_w) / 2;
  f_y = y + (h - f_h) / 2;
  b_x = x + (w - b_w) / 2;
  b_y = y + (h - b_h) / 2;
  draw_absolute_rectangle(b_x, b_y, b_w, b_h, b_color);
  draw_absolute_rectangle(f_x, f_y, f_w, f_h, f_color);
}

void Renderer::draw_active_perk(const Game &game) {
  const int interval = PERK_FADING_INTERVAL;
  const int y_padding = context.settings.get_bar_height();
  const auto remaining = static_cast<int>(game.perk_end_frame - game.played_frames);
  const double fraction = std::min(interval, remaining) / static_cast<double>(interval);
  const int x = game.perk_x;
  const int y = y_padding + game.perk_y;
  draw_resized_perk(x, y, game.get_tile_w(), game.get_tile_h(), fraction);
}

void Renderer::draw_perk(const Game &game) {
  if (has_active_perk(game) != 0) {
    draw_active_perk(game);
  }
}

Code Renderer::draw_player(const Player &player) {
  draw_absolute_tile_rectangle(player.x, player.y, COLOR_PAIR_PLAYER.foreground);
  const auto points = static_cast<double>(player.graphics.get_maximum_size());
  size_t i = 0;
  for (const auto point : player.graphics.trail) {
    auto color = COLOR_PAIR_PLAYER.foreground;
    color.a = static_cast<U8>((i + 1) * (std::numeric_limits<U8>::max() / points));
    draw_shaded_absolute_tile_rectangle(point.x, point.y, color);
    i++;
  }
  return CODE_OK;
}

void Renderer::draw_debugging(const Game &game) {
  const auto height = context.settings.get_bar_height();
  const Color color(255, 255, 255, 127);
  for (int x = game.box.min_x; x <= game.box.max_x; x++) {
    for (int y = game.box.min_y; y <= game.box.max_y; y++) {
      if (get_from_rigid_matrix(&game, x, y) != 0u) {
        draw_absolute_rectangle(x, height + y, 1, 1, color);
      }
    }
  }
}

/**
 * Draws a full game to the screen.
 *
 * Returns a Milliseconds approximation of the time this function took.
 */
Milliseconds Renderer::draw_game(const Game &game) {
  Milliseconds draw_game_start = get_milliseconds();
  auto &profiler = game.context.profiler;

  profiler.start("draw_game");

  profiler.start("clear");
  clear();
  profiler.stop();

  profiler.start("draw_top_bar");
  draw_top_bar(game);
  profiler.stop();

  profiler.start("draw_bottom_bar");
  draw_bottom_bar(game.message);
  profiler.stop();

  profiler.start("draw_platforms");
  draw_platforms(game.platforms, game.box);
  profiler.stop();

  profiler.start("draw_perk");
  draw_perk(game);
  profiler.stop();

  profiler.start("draw_player");
  draw_player(*game.player);
  profiler.stop();

  if (game.debugging) {
    draw_debugging(game);
  }

  profiler.start("present");
  present();
  profiler.stop();

  profiler.stop();
  return get_milliseconds() - draw_game_start;
}

std::string Renderer::record_to_string(const Record &record, const int width) {
  const char format[] = "%s%*.*s%ld";
  const auto name = record.get_name();
  const auto score = record.get_score();
  const auto pad_length = static_cast<int>(width - name.size() - count_digits(score));
  char pad_string[MAXIMUM_STRING_SIZE];
  memset(pad_string, '.', MAXIMUM_STRING_SIZE - 1);
  pad_string[MAXIMUM_STRING_SIZE - 1] = '\0';
  char destination[MAXIMUM_STRING_SIZE];
  sprintf(destination, format, name.c_str(), pad_length, pad_length, pad_string, score);
  return std::string(destination);
}

void Renderer::print_records(const RecordTable &records) {
  const auto pair = COLOR_PAIR_DEFAULT;
  const int x_padding = 2 * context.settings.get_padding() * get_font_width();
  const int y_padding = 2 * context.settings.get_padding() * get_font_height();
  const int available_window_height = context.settings.get_window_height() - y_padding;
  const int text_width_in_pixels = context.settings.get_window_width() - x_padding;
  const size_t string_width = text_width_in_pixels / get_font_width();
  const U32 text_lines_limit = available_window_height / get_font_height();
  const size_t printed = std::min(records.size(), text_lines_limit);
  std::vector<std::string> strings;
  for (size_t i = 0; i < printed; i++) {
    strings.push_back(record_to_string(*(records.begin() + i), string_width));
  }
  clear();
  print_centered_vertically(strings, pair);
  present();
}

Code Renderer::top_scores(CommandTable *table) {
  context.profiler.start("top_scores");
  RecordTable record_table(default_record_table_size);
  record_table.load(default_record_table_filename);
  print_records(record_table);
  context.profiler.stop();
  return wait_for_input(context.settings, table);
}

/**
 * Asserts whether or not a character is a valid input character.
 *
 * For simplicity, the user should only be able to enter letters and numbers.
 */
static bool is_valid_input_character(char character) {
  return isalnum(character);
}

void Renderer::print_limited(const int x, const int y, const char *string, const size_t limit) {
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
    print_absolute(x, y, string, COLOR_PAIR_DEFAULT);
    return;
  }
  /* String is longer than the limit. */
  /* Write the ellipsis if we need to. */
  if (limit >= minimum_string_size_for_ellipsis) {
    print_absolute(x, y, ellipsis, COLOR_PAIR_DEFAULT);
  }
  /* Write the tail of the input string. */
  string += string_length - limit + ellipsis_length;
  print_absolute(x + ellipsis_length, y, string, COLOR_PAIR_DEFAULT);
}

/**
 * Reads a string from the user of up to size characters (including NUL).
 */
Code Renderer::read_string(const int x, const int y, const char *prompt, char *destination, const size_t size) {
  const int buffer_x = x + (strlen(prompt) + 1) * get_font_width();
  const int padding_size = context.settings.get_padding() * get_font_width();
  const int buffer_view_size = context.settings.get_window_width() - buffer_x - padding_size;
  const int buffer_view_limit = buffer_view_size / get_font_width();
  int is_done = 0;
  int should_rerender = 1;
  /* The x coordinate of the user input buffer. */
  auto written = strlen(destination);
  char character;
  auto write = destination + written;
  SDL_Event event{};
  /* Start listening for text input. */
  SDL_StartTextInput();
  while (is_done == 0) {
    if (should_rerender != 0) {
      clear();
      print_absolute(x, y, prompt, COLOR_PAIR_DEFAULT);
      if (written == 0) {
        /* We must write a single space, or SDL will not render anything. */
        print_absolute(buffer_x, y, " ", COLOR_PAIR_DEFAULT);
      } else {
        // Must care about how much we write, padding should be respected.
        // we print the tail of the string and omit the beginning with an ellipsis.
        print_limited(buffer_x, y, destination, buffer_view_limit);
      }
      present();
      should_rerender = 0;
    }
    /* Throughout the loop, the write pointer always points to a '\0'. */
    if (SDL_WaitEvent(&event) != 0) {
      // Check for user quit and return 1.
      // This is OK because the destination string is always a valid C string.
      if (event.type == SDL_QUIT) {
        return CODE_QUIT;
      }
      if (event.type == SDL_KEYDOWN) {
        /* Handle backspace. */
        if (event.key.keysym.sym == SDLK_BACKSPACE && written > 0) {
          write--;
          *write = '\0';
          written--;
          should_rerender = 1;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
          is_done = 1;
        }
      } else if (event.type == SDL_JOYBUTTONDOWN) {
        if (command_from_joystick_event(context.settings, event) == COMMAND_ENTER) {
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
