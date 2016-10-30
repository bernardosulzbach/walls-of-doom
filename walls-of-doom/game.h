#ifndef GAME_H
#define GAME_H

#include "box.h"
#include "clock.h"
#include "code.h"
#include "constants.h"
#include "logger.h"
#include "numeric.h"
#include "perk.h"
#include "platform.h"
#include "player.h"
#include "random.h"
#include "settings.h"
#include <SDL.h>
#include <stdlib.h>

typedef struct Game {

  Player *player;

  Platform *platforms;
  size_t platform_count;

  /**
   * In which frame - starting at 0 - we are now.
   */
  unsigned long frame;

  /**
   * Count of frames on which the player was active (not in the initial state).
   */
  unsigned long played_frames;

  Perk perk;
  int perk_x;
  int perk_y;
  unsigned long perk_end_frame;

  BoundingBox *box;

  size_t rigid_matrix_n;
  size_t rigid_matrix_m;
  size_t rigid_matrix_size;
  unsigned char *rigid_matrix;

  char message[MAXIMUM_STRING_SIZE];
  unsigned long message_end_frame;
  unsigned int message_priority;

} Game;

/**
 * Creates a new Game object with the provided objects.
 */
Game create_game(Player *player, Platform *platforms,
                 const size_t platform_count, BoundingBox *box);

void destroy_game(Game *game);

Milliseconds update_game(Game *const game);

unsigned char get_from_rigid_matrix(const Game *const game, const int x,
                                    const int y);
void modify_rigid_matrix_point(const Game *const game, const int x, const int y,
                               const unsigned char delta);
void modify_rigid_matrix_platform(Game *game, Platform const *platform,
                                  const unsigned char delta);

/**
 * Changes the game message to the provided text, for the provided duration.
 *
 * If there is a message and it has higher priority, it is not changed.
 *
 * This function prevents buffer overflow by truncating the message.
 */
void game_set_message(Game *const game, const char *message,
                      const unsigned long duration,
                      const unsigned int priority);

/**
 * Runs the main game loop for the Game object and registers the player score.
 */
Code run_game(Game *const game, SDL_Renderer *renderer);

#endif
