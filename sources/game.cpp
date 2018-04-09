#include "game.hpp"
#include "analyst.hpp"
#include "io.hpp"
#include "record_table.hpp"
#include "text.hpp"
#include <cstring>

#define DEFAULT_LIMIT_PLAYED_MINUTES 2
#define DEFAULT_LIMIT_PLAYED_SECONDS (DEFAULT_LIMIT_PLAYED_MINUTES * 60)
#define DEFAULT_LIMIT_PLAYED_FRAMES DEFAULT_LIMIT_PLAYED_SECONDS *UPS

static const Milliseconds register_score_release_delay = 200;
static const Milliseconds milliseconds_in_a_second = 1000;
static const int maximum_fps = 250;

static void initialize_rigid_matrix(Game *game) {
  for (size_t i = 0; i < game->platforms.size(); i++) {
    modify_rigid_matrix_platform(game, game->platforms.data() + i, 1);
  }
}

Game::Game(Context &context, Player *player) : context(context), player(player) {
  box.min_x = 0;
  box.min_y = 0;
  box.max_x = context.settings.get_window_width();
  box.max_y = context.settings.get_window_height() - 2 * context.settings.get_bar_height();

  player->w = get_tile_w();
  player->h = get_tile_h();
  reposition_player(this);

  const BoundingBox avoidance{player->x, player->y, player->x + player->w, player->y + player->h};
  platforms = generate_platforms(context.settings, box, avoidance, context.settings.get_platform_count());

  current_frame = 0;
  desired_frame = 0;

  played_frames = 0;
  limit_played_frames = DEFAULT_LIMIT_PLAYED_FRAMES;

  perk = PERK_NONE;
  perk_x = 0;
  perk_y = 0;
  /* Don't start with a Perk on the screen. */
  perk_end_frame = context.settings.get_perk_screen_duration() * UPS;

  rigid_matrix_m = static_cast<size_t>(box.max_y - box.min_y + 1);
  rigid_matrix_n = static_cast<size_t>(box.max_x - box.min_x + 1);
  rigid_matrix.resize(rigid_matrix_m * rigid_matrix_n);
  initialize_rigid_matrix(this);

  message[0] = '\0';
  message_end_frame = 0;
  message_priority = 0;

  log_message("Finished creating the game.");
}

Milliseconds update_game(Game &game) {
  Milliseconds game_update_start;
  game.context.profiler.start("update_game");
  game_update_start = get_milliseconds();
  if (game.message_end_frame < game.current_frame) {
    game.message[0] = '\0';
  }
  update_platforms(&game);
  update_perk(&game);
  game.context.profiler.stop();
  return get_milliseconds() - game_update_start;
}

/**
 * Changes the game message to the provided text, for the provided duration.
 *
 * If there is a message and it has higher priority, it is not changed.
 *
 * This function prevents buffer overflow by truncating the message.
 */
void game_set_message(Game *const game, const char *message, const U64 duration, const unsigned int priority) {
  const auto last_has_expired = static_cast<const int>(game->message_end_frame <= game->current_frame);
  const auto last_has_lower_priority = static_cast<const int>(game->message_priority <= priority);
  if ((last_has_expired != 0) || (last_has_lower_priority != 0)) {
    game->message_end_frame = game->current_frame + duration * UPS;
    game->message_priority = priority;
    copy_string(game->message, message, MAXIMUM_STRING_SIZE);
  }
}

static void print_game_result(const Settings &settings, const Player *player, const U32 position, SDL_Renderer *renderer) {
  const auto name = player->name;
  const Score score = player->score;
  const ColorPair color = COLOR_PAIR_DEFAULT;
  char first_line[MAXIMUM_STRING_SIZE];
  char empty_line[1] = "";
  char second_line[MAXIMUM_STRING_SIZE];
  sprintf(first_line, "%s died after making %ld points.", name.c_str(), score);
  if (position > 0) {
    sprintf(second_line, "%s got to position %d!", name.c_str(), position);
  } else {
    sprintf(second_line, "%s didn't make it to the top scores.", name.c_str());
  }
  std::vector<std::string> lines;
  lines.emplace_back(first_line);
  lines.emplace_back(empty_line);
  lines.emplace_back(second_line);
  clear(renderer);
  print_centered_vertically(settings, lines, color, renderer);
  present(renderer);
}

Code register_score(const Game *const game, SDL_Renderer *renderer) {
  const Player *const player = game->player;
  char buffer[MAXIMUM_STRING_SIZE];
  const char *format = "Started registering a score of %d points for %s.";
  RecordTable table(default_record_table_size);
  table.load(default_record_table_filename);
  const auto position = table.add_record(Record(player->name.c_str(), player->score));
  table.dump(default_record_table_filename);
  sprintf(buffer, format, player->score, player->name.c_str(), renderer);
  log_message(buffer);
  log_message("Saved the record successfully.");
  print_game_result(game->context.settings, player, position, renderer);
  const Milliseconds release_time = get_milliseconds() + register_score_release_delay;
  Code code = CODE_OK;
  while ((code = wait_for_input(game->context.settings, game->player->table)) == CODE_OK) {
    if (get_milliseconds() > release_time) {
      break;
    }
  }
  return code;
}

/**
 * Runs the main game loop for the Game object and registers the player score.
 */
Code run_game(Game *const game, SDL_Renderer *renderer) {
  log_message("Started running a game of difficulty " + double_to_string(get_difficulty(*game), 4) + ".");
  const Milliseconds frame_interval = milliseconds_in_a_second / maximum_fps;
  const Milliseconds logic_interval = milliseconds_in_a_second / UPS;
  Milliseconds start_time = 0;
  Milliseconds time_since_last_logic_update = 0;
  Code code = CODE_OK;
  int *lives = &game->player->lives;
  U64 limit = game->limit_played_frames;
  CommandTable table{};
  initialize_command_table(&table);
  while ((game->player->table->status[COMMAND_QUIT] == 0.0) && *lives != 0 && game->played_frames < limit) {
    start_time = get_milliseconds();
    if (time_since_last_logic_update >= 2 * logic_interval) {
      std::cerr << "Skipped a frame!" << '\n';
    }
    while (time_since_last_logic_update > logic_interval) {
      time_since_last_logic_update -= logic_interval;
      game->desired_frame++;
    }
    if (game->paused) {
      draw_game(game, renderer);
      read_commands(game->context.settings, game->player->table);
      if (test_command_table(game->player->table, COMMAND_CLOSE, REPETITION_DELAY)) {
        code = CODE_CLOSE;
      }
      if (test_command_table(game->player->table, COMMAND_QUIT, REPETITION_DELAY)) {
        code = CODE_QUIT;
      }
      if (test_command_table(game->player->table, COMMAND_PAUSE, REPETITION_DELAY)) {
        game->paused = false;
      }
      continue;
    }
    while (game->current_frame < game->desired_frame) {
      update_game(*game);
      update_player(game, game->player);
      game->current_frame++;
    }
    draw_game(game, renderer);
    read_commands(game->context.settings, game->player->table);
    if (test_command_table(game->player->table, COMMAND_PAUSE, REPETITION_DELAY)) {
      game->paused = true;
    }
    if (test_command_table(game->player->table, COMMAND_DEBUG, REPETITION_DELAY)) {
      game->debugging = !game->debugging;
    }
    const auto time_since_last_frame_update = get_milliseconds() - start_time;
    if (time_since_last_frame_update < frame_interval) {
      sleep_milliseconds(frame_interval - time_since_last_frame_update);
    }
    time_since_last_logic_update += get_milliseconds() - start_time;
  }
  if (code != CODE_CLOSE) {
    code = register_score(game, renderer);
  }
  if (code == CODE_QUIT) {
    /* When the player quits from the game, it should go back to the menu. */
    code = CODE_OK;
  }
  return code;
}
