#include "physics.h"
#include "constants.h"
#include "limits.h"
#include "logger.h"
#include "memory.h"
#include "profiler.h"
#include "random.h"
#include "settings.h"
#include <stdio.h>

/* Should be the maximum frame count value for 5 seconds remaining. */
#define MINIMUM_REMAINING_FRAMES_FOR_MESSAGE (6 * FPS - 1)

/* Extra level of indirection needed to expand macros before the conversion. */
#define AS_STR(X) #X
#define STR(X) AS_STR(X)

#define BUY_LIFE_PRICE 100
#define BUY_LIFE_FORMAT(PRICE) "Bought an extra life for " STR(PRICE) " points."
#define BUY_LIFE_MESSAGE BUY_LIFE_FORMAT(BUY_LIFE_PRICE)

/**
 * Evaluates whether or not a point is within a Platform.
 */
static int is_within_platform(const int x, const int y,
                              const Platform *const platform) {
  const int p_min_x = platform->x;
  const int p_max_x = platform->x + platform->width - 1;
  const int p_y = platform->y;
  return y == p_y && x >= p_min_x && x <= p_max_x;
}

static int is_over_platform(const int x, const int y,
                            const Platform *const platform) {
  return is_within_platform(x, y + 1, platform);
}

/**
 * Evaluates whether or not the given x and y pair is a valid position for the
 * player to occupy.
 */
static int is_valid_move(const Game *const game, const int x, const int y) {
  if (game->player->perk == PERK_POWER_INVINCIBILITY) {
    /* If it is invincible, it shouldn't move into walls. */
    if (game->box->min_x - 1 == x) {
      return 0;
    } else if (game->box->max_x + 1 == x) {
      return 0;
    } else if (game->box->min_y - 1 == y) {
      return 0;
    } else if (game->box->max_y + 1 == y) {
      return 0;
    }
  }
  return !get_from_rigid_matrix(game, x, y);
}

/**
 * Evaluates whether or not an object with the specified speed should move in
 * the current frame of the provided Game.
 *
 * Speed may be any integer, this function is robust enough to handle
 * nonpositive integers.
 */
static int should_move_at_current_frame(const Game *const game,
                                        const int speed) {
  /* Reasoning for rounding a double. */
  /* Let FPS = 30 and speed = 16, if we perform integer division, we will get */
  /* one. This would be much faster than a speed of 16 would actually be as */
  /* ideally the object would be moved at every 1.875 frame. Therefore, it is */
  /* much better to update it at every other frame than at every frame. This */
  /* shows that the expected behavior is reached by rounding a precise */
  /* division rather than by truncating the quotient. */
  /* Play it safe with floating point errors. */
  unsigned long multiple;
  if (speed == 0 || game->frame == 0) {
    return 0;
  } else {
    /* Only divide by abs(speed) after checking that speed != 0. */
    multiple = FPS / (double)abs(speed) + 0.5;
    return game->frame % multiple == 0;
  }
}

static void subtract_platform(Game *const game, Platform *const platform) {
  modify_rigid_matrix_platform(game, platform, -1);
}

static void add_platform(Game *const game, Platform *const platform) {
  modify_rigid_matrix_platform(game, platform, 1);
}

/**
 * Returns whether or not the platform can be inserted in the game without
 * overlapping any existing platforms.
 */
static int can_insert_platform(Game *const game, Platform *const platform) {
  int i;
  for (i = 0; i < platform->width; i++) {
    if (get_from_rigid_matrix(game, platform->x + i, platform->y)) {
      return 0;
    }
  }
  return 1;
}

/**
 * Evaluates whether or not the player is standing on a platform.
 *
 * This function takes into account the Invincibility perk, which makes the
 * bottom border to be treated as a platform.
 */
static int is_standing_on_platform(const Game *const game) {
  if (game->player->y == game->box->max_y) {
    return game->player->perk == PERK_POWER_INVINCIBILITY;
  }
  return get_from_rigid_matrix(game, game->player->x, game->player->y + 1);
}

/**
 * Moves the player by the provided x and y directions. This moves the player
 * at most one position on each axis.
 */
static void move_player(Game *game, int x, int y) {
  /* Ignore magnitude, take just -1, 0, or 1. */
  /* It is good to reuse these variables to prevent mistakes by having */
  /* multiple integers for the same axis. */
  x = normalize(x);
  y = normalize(y);
  if (is_valid_move(game, game->player->x + x, game->player->y + y)) {
    game->player->x += x;
    game->player->y += y;
  }
}

/**
 * Attempts to force the Player to move according to the provided displacement.
 *
 * If the player does not have physics enabled, this is a no-op.
 */
static void shove_player(Game *const game, int x, int y) {
  if (game->player->physics) {
    if (game->player->perk != PERK_POWER_LEVITATION) {
      move_player(game, x, 0);
    }
    move_player(game, 0, y);
  }
}

static int can_move_platform(Game *const game, Platform *const platform,
                             const int dx, const int dy) {
  int can_move;
  int x = game->player->x;
  int y = game->player->y;
  if (get_player_stops_platforms() && is_over_platform(x, y, platform)) {
    return 0;
  }
  /* If the platform would intersect with another platform, do not move it. */
  subtract_platform(game, platform);
  platform->x += dx;
  platform->y += dy;
  can_move = can_insert_platform(game, platform);
  platform->x -= dx;
  platform->y -= dy;
  add_platform(game, platform);
  return can_move;
}

/**
 * This function is the ONLY right way to move a platform.
 *
 * This function keeps the cached rigid body matrix in the Game object valid.
 */
static void move_platform(Game *const game, Platform *const platform,
                          const int dx, const int dy) {
  if (can_move_platform(game, platform, dx, dy)) {
    subtract_platform(game, platform);
    platform->x += dx;
    platform->y += dy;
    add_platform(game, platform);
  }
}

static void move_platform_horizontally(Game *const game,
                                       Platform *const platform) {
  const int normalized_speed = normalize(platform->speed);
  Player *const player = game->player;
  if (should_move_at_current_frame(game, platform->speed)) {
    if (can_move_platform(game, platform, normalized_speed, 0)) {
      /* Fail fast if the platform is not on the same line. */
      if (player->y == platform->y) {
        if (normalized_speed == 1) {
          if (player->x == platform->x + platform->width) {
            shove_player(game, 1, 0);
          }
        } else if (normalized_speed == -1) {
          if (player->x == platform->x - 1) {
            shove_player(game, -1, 0);
          }
        }
      } else if (is_over_platform(player->x, player->y, platform)) {
        shove_player(game, normalized_speed, 0);
      }
      move_platform(game, platform, normalized_speed, 0);
    }
  }
}

/**
 * From an array of lines occupancy states, selects at random an empty line.
 *
 * If no such line exists, returns a random line.
 *
 * This algorithm is O(n) with respect to the number of lines.
 */
int select_random_line_blindly(const unsigned char *lines, const int size) {
  int count;
  int skip;
  int line;
  int i;
  /* Be careful not to call random_integer with invalid parameters. */
  if (size < 1) {
    return 0;
  }
  /* Count how many empty lines there are. */
  count = 0;
  for (i = 0; i < size; i++) {
    if (!lines[i]) {
      count++;
    }
  }
  /* No empty lines, return any line. */
  if (count == 0) {
    return random_integer(0, size - 1);
  }
  /* Get a random value based on the count. */
  skip = random_integer(0, count - 1);
  line = 0;
  while (lines[line] || skip != 0) {
    if (!lines[line]) {
      skip--;
    }
    line = (line + 1) % size;
  }
  return line;
}

/**
 * From an array of lines occupancy states, selects at random one of the lines
 * which are the furthest away from any other occupied line.
 *
 * This algorithm is O(n) with respect to the number of lines.
 */
int select_random_line_awarely(const unsigned char *lines, const int size) {
  int *distances = NULL;
  int maximum_distance = INT_MIN;
  int count;
  int skip;
  int line;
  int i;
  /* Be careful not to call random_integer with invalid parameters. */
  if (size < 1) {
    return 0;
  }
  distances = resize_memory(distances, sizeof(int) * size);
  /* First pass: calculate the distance to nearest occupied line above. */
  for (i = 0; i < size; i++) {
    if (lines[i]) {
      distances[i] = 0;
    } else {
      if (i > 0) {
        distances[i] = distances[i - 1] + 1;
      } else {
        distances[i] = 1;
      }
    }
  }
  /* Second pass: calculate the distance to nearest occupied line below. */
  for (i = size - 1; i >= 0; i--) {
    if (lines[i]) {
      distances[i] = 0;
    } else {
      if (i < size - 1) {
        /* Use the minimum distance to first occupied line above or below. */
        distances[i] = min(distances[i], distances[i + 1] + 1);
      } else {
        distances[i] = 1;
      }
    }
    maximum_distance = max(maximum_distance, distances[i]);
  }
  /* Count how many occurrences of the maximum distance there are. */
  count = 0;
  for (i = 0; i < size; i++) {
    if (distances[i] == maximum_distance) {
      count++;
    }
  }
  /* Get a random value based on the count. */
  skip = random_integer(0, count - 1);
  line = 0;
  while (distances[line] != maximum_distance || skip != 0) {
    if (distances[line] == maximum_distance) {
      skip--;
    }
    line = (line + 1) % size;
  }
  resize_memory(distances, 0);
  return line;
}

static void reposition(Game *const game, Platform *const platform) {
  const BoundingBox *const box = game->box;
  /* The occupied size may be smaller than the array actually is. */
  const int occupied_size = get_lines() - 2;
  unsigned char *occupied = NULL;
  int line;
  size_t i;
  occupied = resize_memory(occupied, occupied_size);
  memset(occupied, 0, occupied_size);
  /* Build a table of occupied rows. */
  for (i = 0; i < game->platform_count; i++) {
    if (!platform_equals(game->platforms[i], *platform)) {
      occupied[game->platforms[i].y - box->min_y] = 1;
    }
  }
  if (get_reposition_algorithm() == REPOSITION_SELECT_BLINDLY) {
    line = select_random_line_blindly(occupied, occupied_size);
  } else {
    line = select_random_line_awarely(occupied, occupied_size);
  }
  resize_memory(occupied, 0);
  if (platform->x > box->max_x) {
    subtract_platform(game, platform);
    /* The platform should be one tick inside the box. */
    platform->x = box->min_x - platform->width + 1;
    platform->y = line + box->min_y;
    add_platform(game, platform);
  } else if (platform->x + platform->width < box->min_x) {
    subtract_platform(game, platform);
    /* The platform should be one tick inside the box. */
    platform->x = box->max_x;
    platform->y = line + box->min_y;
    add_platform(game, platform);
  }
}

/**
 * Evaluates whether or not a Platform is completely outside of a BoundingBox.
 *
 * Returns 0 if the platform intersects the bounding box.
 * Returns 1 if the platform is to the left or to the right of the bounding box.
 * Returns 2 if the platform is above or below the bounding box.
 */
int is_out_of_bounding_box(Platform *const platform,
                           const BoundingBox *const box) {
  const int min_x = platform->x;
  const int max_x = platform->x + platform->width;
  if (max_x < box->min_x || min_x > box->max_x) {
    return 1;
  } else if (platform->y < box->min_y || platform->y > box->max_y) {
    return 2;
  } else {
    return 0;
  }
}

static void update_platform(Game *const game, Platform *const platform) {
  move_platform_horizontally(game, platform);
  if (is_out_of_bounding_box(platform, game->box)) {
    reposition(game, platform);
  }
}

void update_platforms(Game *const game) {
  size_t i;
  if (game->player->perk != PERK_POWER_TIME_STOP) {
    for (i = 0; i < game->platform_count; i++) {
      update_platform(game, game->platforms + i);
    }
  }
}

/**
 * Evaluates whether or not the Player is falling.
 */
static int is_falling(const Game *const game) {
  const Player *const player = game->player;
  const int x = player->x;
  const int y = player->y;
  if (!player->physics || player->perk == PERK_POWER_LEVITATION) {
    return 0;
  }
  if (y == game->box->max_y) {
    return 1;
  }
  return !get_from_rigid_matrix(game, x, y);
}

static int is_touching_a_wall(const Player *const player,
                              const BoundingBox *const box) {
  const int horizontally = player->x < box->min_x || player->x > box->max_x;
  const int vertically = player->y < box->min_y || player->y > box->max_y;
  return horizontally || vertically;
}

static int get_bounding_box_center_x(const BoundingBox *const box) {
  return box->min_x + (box->max_x - box->min_x + 1) / 2;
}

static int get_bounding_box_center_y(const BoundingBox *const box) {
  return box->min_y + (box->max_y - box->min_y + 1) / 2;
}

void reposition_player(Player *const player, const BoundingBox *const box) {
  player->x = get_bounding_box_center_x(box);
  player->y = get_bounding_box_center_y(box);
}

/**
 * Conceives a bonus perk to the player.
 */
void conceive_bonus(Player *const player, Perk perk) {
  if (is_bonus_perk(perk)) {
    if (perk == PERK_BONUS_EXTRA_POINTS) {
      player->score += 60;
    } else if (perk == PERK_BONUS_EXTRA_LIFE) {
      player->lives += 1;
    }
  } else {
    log_message("Called conceive_bonus with a Perk that is not a bonus!");
  }
}

void update_perk(Game *const game) {
  unsigned long next_perk_frame = game->perk_end_frame;
  next_perk_frame += PERK_INTERVAL_IN_FRAMES;
  next_perk_frame -= PERK_SCREEN_DURATION_IN_FRAMES;
  if (game->played_frames == game->perk_end_frame) {
    /* Current Perk (if any) must end. */
    game->perk = PERK_NONE;
  } else if (game->played_frames == next_perk_frame) {
    game->perk = get_random_perk();
    game->perk_x = random_integer(game->box->min_x, game->box->max_x);
    game->perk_y = random_integer(game->box->min_y, game->box->max_y);
    game->perk_end_frame = game->played_frames + PERK_SCREEN_DURATION_IN_FRAMES;
  }
}

/**
 * Moves the player according to the sign of its current speed if it can move
 * in that direction.
 */
void update_player_horizontal_position(Game *game) {
  if (should_move_at_current_frame(game, game->player->speed_x)) {
    if (game->player->speed_x > 0) {
      move_player(game, 1, 0);
    } else if ((game->player->speed_x) < 0) {
      move_player(game, -1, 0);
    }
  }
}

int is_jumping(const Player *const player) {
  return player->remaining_jump_height > 0;
}

void process_jump(Game *const game) {
  if (is_standing_on_platform(game)) {
    game->player->remaining_jump_height = PLAYER_JUMPING_HEIGHT;
    if (game->player->perk == PERK_POWER_SUPER_JUMP) {
      game->player->remaining_jump_height *= 2;
    }
  } else if (game->player->can_double_jump) {
    game->player->can_double_jump = 0;
    game->player->remaining_jump_height += PLAYER_JUMPING_HEIGHT / 2;
    if (game->player->perk == PERK_POWER_SUPER_JUMP) {
      game->player->remaining_jump_height *= 2;
    }
  }
}

static void buy_life(Game *game) {
  if (game->player->score >= BUY_LIFE_PRICE) {
    game->player->score -= BUY_LIFE_PRICE;
    game->player->lives++;
    game_set_message(game, BUY_LIFE_MESSAGE, 1, 1);
  }
}

void process_command(Game *game, const Command command) {
  Player *player = game->player;
  if (command != COMMAND_NONE) {
    player->physics = 1;
  }
  /* Update the player running state */
  if (command == COMMAND_LEFT) {
    if (player->speed_x == 0) {
      player->speed_x = -PLAYER_RUNNING_SPEED;
    } else if (player->speed_x > 0) {
      player->speed_x = 0;
    }
  } else if (command == COMMAND_RIGHT) {
    if (player->speed_x == 0) {
      player->speed_x = PLAYER_RUNNING_SPEED;
    } else if (player->speed_x < 0) {
      player->speed_x = 0;
    }
  } else if (command == COMMAND_JUMP) {
    process_jump(game);
  } else if (command == COMMAND_CONVERT) {
    buy_life(game);
  }
}

/**
 * Checks if the character should die and kills it if this is the case.
 */
static void check_for_player_death(Game *game) {
  Player *player = game->player;
  BoundingBox *box = game->box;
  /* Kill the player if it is touching a wall. */
  if (is_touching_a_wall(player, box)) {
    player->lives--;
    reposition_player(player, box);
    /* Unset physics collisions for the player. */
    player->physics = 0;
    player->speed_x = 0;
    player->can_double_jump = 0;
    player->remaining_jump_height = 0;
  }
}

static int can_move_up(const Game *game) {
  const int x = game->player->x;
  const int y = game->player->y;
  if (y == game->box->min_y) {
    return 1;
  }
  return !get_from_rigid_matrix(game, x, y - 1);
}

/**
 * Updates the vertical position of the player.
 */
void update_player_vertical_position(Game *game) {
  int falling_speed = PLAYER_FALLING_SPEED;
  if (is_jumping(game->player)) {
    if (can_move_up(game)) {
      if (should_move_at_current_frame(game, PLAYER_JUMPING_SPEED)) {
        move_player(game, 0, -1);
        game->player->remaining_jump_height--;
      }
    } else {
      game->player->remaining_jump_height = 0;
    }
  } else if (is_falling(game)) {
    if (game->player->perk == PERK_POWER_LOW_GRAVITY) {
      falling_speed /= 2;
    }
    if (should_move_at_current_frame(game, falling_speed)) {
      move_player(game, 0, 1);
    }
  }
}

void update_double_jump(Game *game) {
  if (is_standing_on_platform(game)) {
    game->player->can_double_jump = 1;
  }
}

static void write_got_perk_message(Game *game, const Perk perk) {
  char message[MAXIMUM_STRING_SIZE];
  sprintf(message, "Got %s!", get_perk_name(perk));
  game_set_message(game, message, 1, 0);
}

static void write_perk_faded_message(Game *game, const Perk perk) {
  char message[MAXIMUM_STRING_SIZE];
  sprintf(message, "%s has faded.", get_perk_name(perk));
  game_set_message(game, message, 1, 0);
}

static void write_perk_fading_message(Game *game, const Perk perk,
                                      const unsigned long remaining_frames) {
  const int seconds = remaining_frames / FPS;
  char message[MAXIMUM_STRING_SIZE];
  const char *perk_name = get_perk_name(perk);
  if (seconds < 1) {
    sprintf(message, "%s will fade at any moment.", perk_name);
  } else if (seconds == 1) {
    sprintf(message, "%s will fade in %d second.", perk_name, seconds);
  } else {
    sprintf(message, "%s will fade in %d seconds.", perk_name, seconds);
  }
  game_set_message(game, message, 1, 0);
}

void update_player_perk(Game *game) {
  unsigned long end_frame;
  unsigned long remaining_frames;
  Player *player = game->player;
  Perk perk = PERK_NONE;
  if (player->physics) {
    game->played_frames++;
    /* Check for expiration of the player's perk. */
    if (player->perk != PERK_NONE) {
      remaining_frames = player->perk_end_frame - game->played_frames;
      if (remaining_frames == 0) {
        write_perk_faded_message(game, player->perk);
        player->perk = PERK_NONE;
      } else if (remaining_frames <= MINIMUM_REMAINING_FRAMES_FOR_MESSAGE) {
        write_perk_fading_message(game, player->perk, remaining_frames);
      }
    }
    if (game->perk != PERK_NONE) {
      if (game->perk_x == player->x && game->perk_y == player->y) {
        /* Copy the Perk to transfer it to the Player */
        perk = game->perk;
        /* Remove the Perk from the screen */
        game->perk = PERK_NONE;
        /* Do not update game->perk_end_frame as it is used to */
        /* calculate when the next perk is going to be created */
        /* Attribute the Perk to the Player */
        player->perk = perk;
        if (is_bonus_perk(perk)) {
          conceive_bonus(player, perk);
          /* The perk ended now. */
          player->perk_end_frame = game->played_frames;
          /* Could set it to the next frame so that the check above */
          /* this part would removed it, but this seems more correct. */
          player->perk = PERK_NONE;
        } else {
          end_frame = game->played_frames + PERK_PLAYER_DURATION_IN_FRAMES;
          player->perk_end_frame = end_frame;
        }
        write_got_perk_message(game, perk);
      }
    }
  }
}

void update_player(Game *game, const Command command) {
  profiler_begin("update_player");
  update_player_perk(game);
  process_command(game, command);
  /* This ordering makes the player run horizontally before falling.
   * This seems to be the expected order from an user point-of-view. */
  update_player_horizontal_position(game);
  /* After moving, if it even happened, simulate jumping and falling. */
  update_player_vertical_position(game);
  /* Enable double jump if the player is standing over a platform. */
  update_double_jump(game);
  check_for_player_death(game);
  profiler_end("update_player");
}
