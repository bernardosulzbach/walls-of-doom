#ifndef GAME_H
#define GAME_H

#include "box.hpp"
#include "clock.hpp"
#include "code.hpp"
#include "constants.hpp"
#include "context.hpp"
#include "integers.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "numeric.hpp"
#include "perk.hpp"
#include "platform.hpp"
#include "player.hpp"
#include "profiler.hpp"
#include "random.hpp"
#include "settings.hpp"
#include <SDL.h>
#include <cstdlib>
#include <vector>

class Game {
public:
  Context &context;

  Player *player;

  std::vector<Platform> platforms;

  U64 current_frame;
  U64 desired_frame;

  // Count of frames on which the player was active (not in the initial state).
  U64 played_frames;
  U64 limit_played_frames;

  bool paused = false;
  bool debugging = false;

  Perk perk;

  int perk_x;
  int perk_y;

  U64 perk_end_frame;

  BoundingBox box;

  size_t rigid_matrix_n;
  size_t rigid_matrix_m;

  std::vector<U8> rigid_matrix;

  char message[MAXIMUM_STRING_SIZE]{};
  U64 message_end_frame;
  unsigned int message_priority;

  Game(Context &context, Player *player);

  inline S32 get_tile_w() const {
    return context.settings.get_tile_w();
  }

  inline S32 get_tile_h() const {
    return context.settings.get_tile_h();
  }
};

Milliseconds update_game(Game &game);

inline size_t get_rigid_matrix_index(const Game *const game, const int x, const int y) {
  const int base_x = x - game->box.min_x;
  const int base_y = y - game->box.min_y;
  return base_x + base_y * game->rigid_matrix_n;
}

inline unsigned char get_from_rigid_matrix(const Game *const game, const int x, const int y) {
  if (game->box.contains(x, y)) {
    return game->rigid_matrix[get_rigid_matrix_index(game, x, y)];
  }
  return 0;
}

inline void modify_rigid_matrix_point(Game *const game, const int x, const int y, S8 delta) {
  if (game->box.contains(x, y)) {
    safe_add(game->rigid_matrix[get_rigid_matrix_index(game, x, y)], delta);
  }
}

inline void modify_rigid_matrix_platform(Game *game, Platform const *platform, S8 delta) {
  for (int x = 0; x < platform->w; ++x) {
    for (int y = 0; y < platform->h; ++y) {
      modify_rigid_matrix_point(game, platform->x + x, platform->y + y, delta);
    }
  }
}

/**
 * Changes the game message to the provided text, for the provided duration.
 *
 * If there is a message and it has higher priority, it is not changed.
 *
 * This function prevents buffer overflow by truncating the message.
 */
void game_set_message(Game *const game, const char *message, const U64 duration, const unsigned int priority);

/**
 * Runs the main game loop for the Game object and registers the player score.
 */
Code run_game(Renderer &renderer, Game &game);

#endif
