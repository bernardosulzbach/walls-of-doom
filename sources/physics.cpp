#include "physics.hpp"

/* Should be the maximum frame count value for 5 seconds remaining. */
#define MINIMUM_REMAINING_FRAMES_FOR_MESSAGE (6 * UPS - 1)

/* Extra level of indirection needed to expand macros before the conversion. */
#define AS_STR(X) #X
#define STR(X) AS_STR(X)

#define BUY_LIFE_PRICE 100
#define BUY_LIFE_FORMAT(PRICE) "Bought an extra life for " STR(PRICE) " points."
#define BUY_LIFE_MESSAGE BUY_LIFE_FORMAT(BUY_LIFE_PRICE)

enum class ShoveResult { ShoveFailure, ShoveSuccess };

static BoundingBox derive_box(const Game *game, const int x, const int y) {
  BoundingBox box;
  box.min_x = x;
  box.min_y = y;
  box.max_x = box.min_x + game->get_tile_w() - 1;
  box.max_y = box.min_y + game->get_tile_h() - 1;
  return box;
}

static bool has_rigid_support(const Game *game, int x, int y, int w, int h) {
  for (int i = 0; i < w; i++) {
    if (get_from_rigid_matrix(game, x + i, y + h) != 0u) {
      return true;
    }
  }
  return false;
}

static bool is_over_platform(const Player *player, const Platform *const platform) {
  if (player->y + player->h == platform->y) {
    if (player->x < platform->x + platform->w) {
      return player->x + player->w > platform->x;
    }
  }
  return false;
}

/* Width and height are the width and height of the matrix tile. */
static bool violates_rigid_matrix(const Game *game, int x, int y, int w, int h) {
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      if (get_from_rigid_matrix(game, x + i, y + j) != 0) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Evaluates whether or not the given x and y pair is a valid position for the player to occupy.
 */
static bool is_valid_move(const Game *const game, const int x, const int y) {
  if (game->player->perk == PERK_POWER_INVINCIBILITY) {
    /* If it is invincible, it shouldn't move into walls. */
    if (x == game->box.min_x - 1) {
      return false;
    }
    if (x + game->player->w - 1 == game->box.max_x + 1) {
      return false;
    }
    if (y == game->box.min_y - 1) {
      return false;
    }
    if (y + game->player->h - 1 == game->box.max_y + 1) {
      return false;
    }
  }
  return !violates_rigid_matrix(game, x, y, game->player->w, game->player->h);
}

/**
 * Moves the player by the provided x and y directions.
 *
 * This moves the player at most one position on each axis.
 */
static void move_player(Game *game, int dx, int dy) {
  // It is OK to reuse x and y to prevent multiple integers for the same axis.
  // Ignore magnitude, take just -1, 0, or 1.
  dx = normalize(dx);
  dy = normalize(dy);
  // Just in case a compiler cannot optimize this case away.
  if (dx == 0 && dy == 0) {
    return;
  }
  if (is_valid_move(game, game->player->x + dx, game->player->y + dy)) {
    game->player->x += dx;
    game->player->y += dy;
  }
}

static bool can_move_player_without_intersecting(Game *game, int dx, int dy) {
  const S32 tile_w = game->context.settings.get_tile_w();
  const S32 tile_h = game->context.settings.get_tile_h();
  for (auto x = game->player->x + dx; x < game->player->x + dx + tile_w; x++) {
    for (auto y = game->player->y + dy; y < game->player->y + dy + tile_h; y++) {
      if (get_from_rigid_matrix(game, x, y) != 0u) {
        return false;
      }
    }
  }
  return true;
}

/**
 * Attempts to force the Player to move according to the provided displacement.
 *
 * If the player does not have physics enabled, this is a no-op.
 *
 * The standing flag indicates if the player is standing above the platform.
 */
static ShoveResult shove_player(Game *game, int dx, int dy, bool standing) {
  if (game->player->physics) {
    // Don't shove the player if he is hovering over a platform.
    if (game->player->perk != PERK_POWER_LEVITATION || !standing) {
      move_player(game, dx, 0);
    }
    // Don't shove if the player would get into a solid object.
    if (!can_move_player_without_intersecting(game, dx, dy)) {
      return ShoveResult::ShoveFailure;
    }
  }
  move_player(game, 0, dy);
  return ShoveResult::ShoveSuccess;
}

static int get_absolute_pending_movement(U64 frame, int speed) {
  // Should move slice after every frame.
  const auto slice = speed / static_cast<double>(UPS);
  // To reduce floating point error, normalize frame to [FPS, 2 FPS - 1].
  frame = frame % UPS + UPS;
  return static_cast<int>(std::floor(frame * slice) - std::floor((frame - 1) * slice));
}

static int get_pending_movement(const Game *const game, const int speed) {
  return normalize(speed) * get_absolute_pending_movement(game->current_frame, abs(speed));
}

static void subtract_platform(Game *const game, Platform *const platform) {
  modify_rigid_matrix_platform(game, platform, -1);
}

static void add_platform(Game *const game, Platform *const platform) {
  modify_rigid_matrix_platform(game, platform, 1);
}

static bool is_free_on_matrix(Game *const game, int x, int y, int w, int h) {
  for (int i = 0; i != w; i++) {
    for (int j = 0; j != h; j++) {
      if (get_from_rigid_matrix(game, x + i, y + j) != 0u) {
        return false;
      }
    }
  }
  return true;
}

static bool can_insert_platform(Game *const game, Platform *const p) {
  return is_free_on_matrix(game, p->x, p->y, p->w, p->h);
}

/**
 * Evaluates whether or not the player is standing on a platform.
 *
 * This function takes into account the Invincibility perk, which makes the bottom border to be treated as a platform.
 */
static bool is_standing_on_platform(const Game *const game) {
  const int x = game->player->x;
  const int y = game->player->y;
  const int w = game->player->w;
  const int h = game->player->h;
  if (y + h - 1 == game->box.max_y) {
    return game->player->perk == PERK_POWER_INVINCIBILITY;
  }
  return has_rigid_support(game, x, y, w, h);
}

static bool is_in_front_of_platform(const Player *const player, const Platform *const platform) {
  if (platform->speed < 0) {
    if (player->x + player->w != platform->x) {
      return false;
    }
  } else {
    if (platform->x + platform->w != player->x) {
      return false;
    }
  }
  if (player->y < platform->y + platform->h) {
    if (player->y + player->h > platform->y) {
      return true;
    }
  }
  return false;
}

static bool can_move_platform(Game *const game, Platform *p, int dx, int dy) {
  if (game->context.settings.get_player_stops_platforms() && is_over_platform(game->player, p)) {
    return false;
  }
  if (dx == 0 && dy == 0) {
    return true;
  }
  auto can_move = true;
  // There are two optimized paths for unidirectional movement.
  if (dx == 0) {
    if (dy < 0) {
      return is_free_on_matrix(game, p->x, p->y + dy, p->w, -dy);
    }
    if (dy > 0) {
      return is_free_on_matrix(game, p->x, p->y + p->h, p->w, dy);
    }
  } else if (dy == 0) {
    if (dx < 0) {
      return is_free_on_matrix(game, p->x + dx, p->y, -dx, p->h);
    }
    if (dx > 0) {
      return is_free_on_matrix(game, p->x + p->w, p->y, dx, p->h);
    }
  } else {
    // If the platform would intersect with another platform, do not move it.
    subtract_platform(game, p);
    p->x += dx;
    p->y += dy;
    can_move = can_insert_platform(game, p);
    p->x -= dx;
    p->y -= dy;
    add_platform(game, p);
  }
  return can_move;
}

static void slide_platform_on_x(Game *const game, Platform *const p, const int dx) {
  if (dx == 0) {
    throw std::logic_error("Bad call.");
  }
  const auto size = std::min(p->w, std::abs(dx));
  int subB = 0;
  int addB = 0;
  if (dx > 0) {
    subB = p->x;
    addB = p->x + std::max(p->w, dx);
  } else {
    subB = std::max(p->x, p->x + p->w - size);
    addB = p->x + dx;
  }
  for (int x = subB; x < subB + size; x++) {
    for (int y = p->y; y < p->y + p->h; y++) {
      modify_rigid_matrix_point(game, x, y, -1);
    }
  }
  for (int x = addB; x < addB + size; x++) {
    for (int y = p->y; y < p->y + p->h; y++) {
      modify_rigid_matrix_point(game, x, y, +1);
    }
  }
  p->x += dx;
}

/**
 * This function is the ONLY right way to move a platform.
 *
 * This function keeps the cached rigid body matrix in the Game object valid.
 */
static void move_platform(Game *const game, Platform *const platform, const int dx, const int dy) {
  if (can_move_platform(game, platform, dx, dy)) {
    slide_platform_on_x(game, platform, dx);
    if (dy != 0) {
      throw std::logic_error("Not implemented.");
    }
  }
}

static void move_platform_horizontally(Game *const game, Platform *const platform) {
  int normalized_speed = normalize(platform->speed);
  /* This could be made more efficient by handling each direction separately. */
  int pending = abs(platform->speed);
  while (pending != 0) {
    if (can_move_platform(game, platform, normalized_speed, 0)) {
      if (is_in_front_of_platform(game->player, platform)) {
        const auto shove_result = shove_player(game, normalized_speed, 0, false);
        if (shove_result == ShoveResult::ShoveFailure) {
          break;
        }
      }
      if (is_over_platform(game->player, platform)) {
        shove_player(game, normalized_speed, 0, true);
      }
      move_platform(game, platform, normalized_speed, 0);
    }
    pending--;
  }
}

int select_random_line_blindly(const std::vector<unsigned char> &lines) {
  if (lines.empty()) {
    throw std::logic_error("Empty line vector.");
  }
  /* Count how many empty lines there are. */
  int count = 0;
  for (const auto line : lines) {
    if (line == 0) {
      count++;
    }
  }
  /* No empty lines, return any line. */
  if (count == 0) {
    return random_integer(0, static_cast<int>(lines.size() - 1));
  }
  /* Get a random value based on the count. */
  int skip = random_integer(0, count - 1);
  int line = 0;
  while ((lines[line] != 0) || skip != 0) {
    if (lines[line] == 0) {
      skip--;
    }
    line = (line + 1) % static_cast<int>(lines.size());
  }
  return line;
}

int select_random_line_awarely(const std::vector<unsigned char> &lines) {
  if (lines.empty()) {
    throw std::logic_error("Empty line vector.");
  }
  auto maximum_distance = std::numeric_limits<int>::min();
  std::vector<int> distances(static_cast<size_t>(lines.size()));
  /* First pass: calculate the distance to nearest occupied line above. */
  for (size_t i = 0; i < lines.size(); i++) {
    if (lines[i] != 0) {
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
  for (int i = static_cast<int>(lines.size()) - 1; i >= 0; i--) {
    if (lines[i] != 0) {
      distances[i] = 0;
    } else {
      if (i < static_cast<int>(lines.size()) - 1) {
        /* Use the minimum distance to first occupied line above or below. */
        distances[i] = std::min(distances[i], distances[i + 1] + 1);
      } else {
        distances[i] = 1;
      }
    }
    maximum_distance = std::max(maximum_distance, distances[i]);
  }
  /* Count how many occurrences of the maximum distance there are. */
  int count = 0;
  for (size_t i = 0; i < lines.size(); i++) {
    if (distances[i] == maximum_distance) {
      count++;
    }
  }
  /* Get a random value based on the count. */
  int skip = random_integer(0, count - 1);
  int line = 0;
  while (distances[line] != maximum_distance || skip != 0) {
    if (distances[line] == maximum_distance) {
      skip--;
    }
    line = (line + 1) % static_cast<int>(lines.size());
  }
  return line;
}

static void reposition(Game *const game, Platform *const platform) {
  const auto box = game->box;
  // The occupied size may be smaller than the array actually is.
  const auto &settings = game->context.settings;
  const auto bar_height = settings.get_bar_height();
  const auto tile_h = game->get_tile_h();
  const auto occupied_size = static_cast<U32>((settings.get_window_height() - 2 * bar_height) / tile_h);
  std::vector<U8> occupied(occupied_size);
  /* Build a table of occupied rows. */
  for (size_t i = 0; i < game->platforms.size(); i++) {
    if (game->platforms[i] != *platform) {
      occupied[(game->platforms[i].y - box.min_y) / tile_h] = 1;
    }
  }
  int line;
  if (game->context.settings.get_reposition_algorithm() == REPOSITION_SELECT_BLINDLY) {
    line = select_random_line_blindly(occupied);
  } else {
    line = select_random_line_awarely(occupied);
  }
  if (platform->x > box.max_x) {
    subtract_platform(game, platform);
    /* The platform should be one tick inside the box. */
    platform->x = box.min_x - platform->w + 1;
    platform->y = box.min_y + tile_h * line;
    add_platform(game, platform);
  } else if (platform->x + platform->w < box.min_x) {
    subtract_platform(game, platform);
    /* The platform should be one tick inside the box. */
    platform->x = box.max_x;
    platform->y = box.min_y + tile_h * line;
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
int is_out_of_bounding_box(Platform *const platform, const BoundingBox *const box) {
  const int min_x = platform->x;
  const int max_x = platform->x + platform->w;
  if (max_x < box->min_x || min_x > box->max_x) {
    return 1;
  }
  if (platform->y < box->min_y || platform->y > box->max_y) {
    return 2;
  }
  return 0;
}

static void update_platform(Game *const game, Platform *const platform) {
  move_platform_horizontally(game, platform);
  if (is_out_of_bounding_box(platform, &game->box) != 0) {
    reposition(game, platform);
  }
}

void update_platforms(Game *const game) {
  if (game->player->perk != PERK_POWER_TIME_STOP) {
    for (size_t i = 0; i < game->platforms.size(); i++) {
      update_platform(game, game->platforms.data() + i);
    }
  }
}

static bool is_falling(const Game *const game) {
  if (!game->player->physics || game->player->perk == PERK_POWER_LEVITATION) {
    return false;
  }
  if (game->player->y == game->box.max_y) {
    return true;
  }
  return !has_rigid_support(game, game->player->x, game->player->y, game->player->w, game->player->h);
}

static bool is_touching_a_wall(const Game *const game) {
  const auto b_min_x = game->box.min_x;
  const auto b_max_x = game->box.max_x;
  const auto b_min_y = game->box.min_y;
  const auto b_max_y = game->box.max_y;
  const auto in_x = game->player->x < b_min_x || game->player->x + game->player->w - 1 > b_max_x;
  const auto in_y = game->player->y < b_min_y || game->player->y + game->player->h - 1 > b_max_y;
  return in_x || in_y;
}

static int get_bounding_box_center_x(const BoundingBox *const box) {
  return box->min_x + (box->max_x - box->min_x + 1) / 2;
}

static int get_bounding_box_center_y(const BoundingBox *const box) {
  return box->min_y + (box->max_y - box->min_y + 1) / 2;
}

void reposition_player(Game *const game) {
  game->player->x = get_bounding_box_center_x(&game->box);
  game->player->y = get_bounding_box_center_y(&game->box);
}

/**
 * Conceives a bonus perk to the player.
 */
void conceive_bonus(Player *const player, const Perk perk) {
  if (is_bonus_perk(perk)) {
    if (perk == PERK_BONUS_EXTRA_POINTS) {
      player->increment_score(EXTRA_POINTS_AMOUNT);
    } else if (perk == PERK_BONUS_EXTRA_LIFE) {
      player->lives += 1;
    }
  } else {
    log_message("Called conceive_bonus with a Perk that is not a bonus!");
  }
}

static void accelerate_platform(Platform *const platform) {
  platform->speed = platform->speed + platform->speed / 2;
}

static void reverse_platform(Platform *const platform) {
  platform->speed = -platform->speed;
}

static void apply_to_platforms(Game *const game, void (*f)(Platform *const)) {
  for (size_t i = 0; i != game->platforms.size(); ++i) {
    f(game->platforms.data() + i);
  }
}

/**
 * Process the start of a curse.
 */
void process_curse(Game *const game, const Perk perk) {
  if (is_curse_perk(perk)) {
    if (perk == PERK_CURSE_ACCELERATE_PLATFORMS) {
      apply_to_platforms(game, accelerate_platform);
    } else if (perk == PERK_CURSE_REVERSE_PLATFORMS) {
      apply_to_platforms(game, reverse_platform);
    }
  } else {
    log_message("Called process_curse with a Perk that is not a curse!");
  }
}

void update_perk(Game *const game) {
  auto next_perk_frame = game->perk_end_frame;
  next_perk_frame += game->context.settings.get_perk_interval() * UPS;
  next_perk_frame -= game->context.settings.get_perk_screen_duration() * UPS;
  if (game->played_frames == game->perk_end_frame) {
    game->perk = PERK_NONE;
  } else if (game->played_frames == next_perk_frame) {
    game->perk = get_random_perk();
    game->perk_x = random_integer(0, game->context.settings.get_window_width() - game->context.settings.get_tile_w());
    const auto bar_height = game->context.settings.get_bar_height();
    const auto random_y = random_integer(bar_height, game->context.settings.get_window_height() - 2 * bar_height);
    game->perk_y = random_y - random_y % game->context.settings.get_tile_h();
    game->perk_end_frame = game->played_frames + game->context.settings.get_perk_screen_duration() * UPS;
  }
}

/**
 * Moves the player according to the sign of its current speed if it can move in that direction.
 */
void update_player_horizontal_position(Game *game) {
  int pending_movement = get_pending_movement(game, game->player->speed_x);
  while (pending_movement > 0) {
    move_player(game, 1, 0);
    pending_movement--;
  }
  while (pending_movement < 0) {
    move_player(game, -1, 0);
    pending_movement++;
  }
}

static bool is_jumping(const Player *const player) {
  return player->remaining_jump_height > 0;
}

void process_jump(Game *const game) {
  const int jumping_height = game->get_tile_h() * PLAYER_JUMPING_HEIGHT;
  if (is_standing_on_platform(game)) {
    game->player->remaining_jump_height = jumping_height;
    if (game->player->perk == PERK_POWER_SUPER_JUMP) {
      game->player->remaining_jump_height *= 2;
    }
  } else if (game->player->can_double_jump != 0) {
    game->player->can_double_jump = 0;
    game->player->remaining_jump_height += jumping_height / 2;
    if (game->player->perk == PERK_POWER_SUPER_JUMP) {
      game->player->remaining_jump_height *= 2;
    }
  }
}

static void buy_life(Game *game) {
  if (game->player->score >= BUY_LIFE_PRICE) {
    game->player->decrement_score(BUY_LIFE_PRICE);
    game->player->lives++;
    game_set_message(game, BUY_LIFE_MESSAGE, 1, 1);
  }
}

void process_command(Game *game, Player *player) {
  double *table = player->table->status;
  if (table[COMMAND_LEFT] != 0.0) {
    double speed = -table[COMMAND_LEFT] * PLAYER_RUNNING_SPEED * game->get_tile_w();
    player->speed_x = static_cast<int>(speed);
    player->physics = true;
  } else if (table[COMMAND_RIGHT] != 0.0) {
    double speed = table[COMMAND_RIGHT] * PLAYER_RUNNING_SPEED * game->get_tile_w();
    player->speed_x = static_cast<int>(speed);
    player->physics = true;
  } else {
    player->speed_x = 0;
  }
  if (table[COMMAND_JUMP] != 0.0) {
    process_jump(game);
    table[COMMAND_JUMP] = 0.0;
    player->physics = true;
  } else if (table[COMMAND_CONVERT] != 0.0) {
    buy_life(game);
    table[COMMAND_CONVERT] = 0.0;
  }
}

/**
 * Checks if the character should die and kills it if this is the case.
 */
static void check_for_player_death(Game *game) {
  Player *player = game->player;
  /* Kill the player if it is touching a wall. */
  if (is_touching_a_wall(game)) {
    player->lives--;
    reposition_player(game);
    /* Unset physics collisions for the player. */
    player->physics = false;
    player->speed_x = 0;
    player->can_double_jump = 0;
    player->remaining_jump_height = 0;
  }
}

static bool can_move_up(const Game *game) {
  const int x = game->player->x;
  const int y = game->player->y;
  if (y == game->box.min_y) {
    return true;
  }
  for (S32 i = 0; i < static_cast<S32>(game->context.settings.get_tile_w()); i++) {
    if (get_from_rigid_matrix(game, x + i, y - 1) != 0u) {
      return false;
    }
  }
  return true;
}

/**
 * Updates the vertical position of the player.
 */
void update_player_vertical_position(Game *game) {
  const int jumping_speed = PLAYER_JUMPING_SPEED * game->get_tile_h();
  const int falling_speed = PLAYER_FALLING_SPEED * game->get_tile_h();
  if (is_jumping(game->player)) {
    if (can_move_up(game)) {
      int pending = get_pending_movement(game, jumping_speed);
      while (pending > 0) {
        move_player(game, 0, -1);
        game->player->remaining_jump_height--;
        pending--;
      }
    } else {
      game->player->remaining_jump_height = 0;
    }
  } else if (is_falling(game)) {
    int pending = 0;
    if (game->player->perk == PERK_POWER_FEATHER_FALL) {
      pending = get_pending_movement(game, falling_speed / 2);
    } else {
      pending = get_pending_movement(game, falling_speed);
    }
    while (pending > 0) {
      move_player(game, 0, 1);
      pending--;
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
  sprintf(message, "Got %s!", get_perk_name(perk).c_str());
  game_set_message(game, message, 1, 0);
}

static void write_perk_faded_message(Game *game, const Perk perk) {
  char message[MAXIMUM_STRING_SIZE];
  sprintf(message, "%s has faded.", get_perk_name(perk).c_str());
  game_set_message(game, message, 1, 0);
}

static void write_perk_fading_message(Game *game, const Perk perk, const U64 remaining_frames) {
  const U64 seconds = remaining_frames / UPS;
  char message[MAXIMUM_STRING_SIZE];
  const char *perk_name = get_perk_name(perk).c_str();
  if (seconds < 1) {
    sprintf(message, "%s will fade at any moment.", perk_name);
  } else if (seconds == 1) {
    sprintf(message, "%s will fade in %lu second.", perk_name, seconds);
  } else {
    sprintf(message, "%s will fade in %lu seconds.", perk_name, seconds);
  }
  game_set_message(game, message, 1, 0);
}

static bool is_touching_perk(const Game *const game) {
  const auto player_box = derive_box(game, game->player->x, game->player->y);
  const auto perk_box = derive_box(game, game->perk_x, game->perk_y);
  return player_box.overlaps(perk_box);
}

static void update_player_perk(Game *game) {
  U64 end_frame;
  U64 remaining_frames;
  Player *player = game->player;
  Perk perk;
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
      if (is_touching_perk(game)) {
        /* Copy the Perk to transfer it to the Player */
        perk = game->perk;
        /* Remove the Perk from the screen */
        game->perk = PERK_NONE;
        /* Do not update game->perk_end_frame as it is used to */
        /* calculate when the next perk is going to be created */
        /* Attribute the Perk to the Player */
        player->perk = perk;
        if ((is_bonus_perk(perk)) || (is_curse_perk(perk))) {
          if (is_bonus_perk(perk)) {
            conceive_bonus(player, perk);
          } else {
            process_curse(game, perk);
          }
          /* The perk ended now. */
          player->perk_end_frame = game->played_frames;
          /* Could set it to the next frame so that the check above */
          /* this part would removed it, but this seems more correct. */
          player->perk = PERK_NONE;
        } else {
          end_frame = game->played_frames + game->context.settings.get_perk_screen_duration() * UPS;
          player->perk_end_frame = end_frame;
        }
        write_got_perk_message(game, perk);
      }
    }
  }
}

static void update_player_graphics(Game *game) {
  game->player->graphics.update_trail(game->player->x, game->player->y);
}

void update_player(Game *game, Player *player) {
  game->context.profiler.start("update_player");
  if (player->physics) {
    log_player_score(game->context.settings, game->played_frames, player->score);
  }
  update_player_graphics(game);
  update_player_perk(game);
  process_command(game, player);
  // This ordering makes the player run horizontally before falling.
  // This seems to be the expected order from an user point-of-view.
  update_player_horizontal_position(game);
  /* After moving, if it even happened, simulate jumping and falling. */
  update_player_vertical_position(game);
  /* Enable double jump if the player is standing over a platform. */
  update_double_jump(game);
  check_for_player_death(game);
  if (is_standing_on_platform(game)) {
    for (const auto &platform : game->platforms) {
      if (platform.y == player->y + player->h) {
        if (player->x < platform.x + platform.w) {
          if (player->x + player->w > platform.x) {
            player->increment_score_from_event(platform.rarity);
          }
        }
      }
    }
  }
  game->context.profiler.stop();
}
