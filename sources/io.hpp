#ifndef BASE_IO_H
#define BASE_IO_H

#include "clock.hpp"
#include "code.hpp"
#include "color.hpp"
#include "command.hpp"
#include "context.hpp"
#include "perk.hpp"
#include "platform.hpp"
#include "record.hpp"
#include "record_table.hpp"
#include <SDL_ttf.h>
#include <string>
#include <vector>

class Game;

extern const char *const game_name;

class Renderer {
public:
  explicit Renderer(Context &context);

  virtual ~Renderer();

  /**
   * Clears the screen.
   */
  void clear();

  /**
   * Updates the screen with what has been rendered.
   */
  void present();

  TTF_Font *get_font();

  int get_font_width();

  int get_font_height();

  void set_color(Color color);

  /**
   * Reads a string from the user of up to size characters (including NUL).
   *
   * The string will be echoed after the prompt, which starts at (x, y).
   */
  Code read_string(int x, int y, const char *prompt, char *destination, size_t size);

  void print_menu(const std::vector<std::string> &lines);

  Code read_player_name(std::string &destination);

  Code print_absolute(int x, int y, const std::string &string, ColorPair color);

  void remove_first_breaks(std::string &string);

  Code print_centered_horizontally(int y, const std::vector<std::string> &strings, ColorPair color);

  Code print_centered_vertically(const std::vector<std::string> &strings, ColorPair color);

  bool is_valid_player_name(const std::string &player_name);

  void draw_absolute_rectangle(int x, int y, int w, int h, Color color);

  void draw_shaded_absolute_rectangle(int x, int y, int w, int h, Color color);

  void draw_absolute_tile_rectangle(int x, int y, Color color);

  void draw_shaded_absolute_tile_rectangle(int x, int y, Color color);

  void write_top_bar_strings(const std::vector<std::string> &strings);

  void draw_top_bar(const Game &game);

  void write_bottom_bar_string(const std::string &string);

  void draw_bottom_bar(const std::string &message);

  Color get_platform_color(Platform platform);

  void draw_platforms(const std::vector<Platform> &platforms, BoundingBox box);

  bool has_active_perk(const Game &game);

  void draw_resized_perk(int x, int y, int w, int h, double f);

  void draw_active_perk(const Game &game);

  void draw_perk(const Game &game);

  Code draw_player(const Player &player);

  void draw_debugging(const Game &game);

  /**
   * Draws a full game to the screen.
   *
   * Returns a Milliseconds approximation of the time this function took.
   */
  Milliseconds draw_game(const Game &game);

  std::string record_to_string(const Record &record, int width);

  void print_records(const RecordTable &records);

  Code top_scores(CommandTable *table);

  /**
   * Prints a string starting from (x, y) but limits to its first limit characters.
   */
  void print_limited(int x, int y, const char *string, size_t limit);

  void print_long_text(std::string string);

private:
  Context &context;
  SDL_Window *window;
  SDL_Renderer *renderer;

  /**
   * Initializes the required resources.
   *
   * Should only be called once, right after starting.
   */
  Code initialize(Settings &settings);

  Code initialize_fonts(const Settings &settings);

  Code initialize_font_metrics();

  void create_window(const Settings &settings, int *width, int *height);

  Code set_window_title_and_icon();

  /**
   * Finalizes the acquired resources.
   *
   * Should only be called once, right before exiting.
   */
  Code finalize();

  void finalize_fonts();
};

#endif
