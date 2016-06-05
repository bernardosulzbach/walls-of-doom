#include "constants.h"
#include "logger.h"
#include "physics.h"
#include "random.h"
#include "vector.h"

#include <stdio.h>

#define PLAYER_RUNNING_SPEED 4
#define PLAYER_FALLING_SPEED 8
#define PLAYER_JUMPING_SPEED PLAYER_FALLING_SPEED

#define PLAYER_JUMPING_HEIGHT 2 * PLAYER_JUMPING_SPEED

/**
 * Repositions a Platform in the vicinity of a BoundingBox.
 */
void reposition(Game * const game, Platform * const platform);

/**
 * Evaluates whether or not a Platform is completely outside of a BoundingBox.
 *
 * Returns 0 if the platform intersects the bounding box.
 * Returns 1 if the platform is to the left or to the right of the bounding box.
 * Returns 2 if the platform is above or below the bounding box.
 */
int is_out_of_bounding_box(Platform * const platform, const BoundingBox * const box);

/**
 * Evaluates whether or not a point is within a Platform.
 */
int is_within_platform(const int x, const int y, const Platform * const platform);

void update_platform(Game * const game, Platform * const platform);

void log_if_not_normalized(const int value) {
    if (value < -1 && value > 1) {
        char buffer[256];
        char *format = "Expected a normalized value, but got %d instead\n";
        sprintf(buffer, format, value);
        log_message(buffer);
    }
}

int bounding_box_equals(const BoundingBox * const a, const BoundingBox * const b) {
    return a->min_x == b->min_x
        && a->min_y == b->min_y
        && a->max_x == b->max_x
        && a->max_y == b->max_y;
}

/**
 * Evaluates whether or not a point is within a Platform.
 */
int is_within_platform(const int x, const int y, const Platform * const platform) {
    return y == platform->y && x >= platform->x && x < platform->x + platform->width;
}

int is_over_platform(const int x, const int y, const Platform * const platform) {
    return is_within_platform(x, y + 1, platform);
}

/**
 * Attempts to force the Player to move according to the provided displacement.
 *
 * If the player does not have physics enabled, this is a no-op.
 */
void shove_player(Player * const player, const Vector displacement) {
    if (player->physics) {
        player->x += displacement.x;
        player->y += displacement.y;
    }
}

/**
 * Evaluates whether or not an object with the specified speed should move in
 * the current frame of the provided Game.
 *
 * Speed may be any integer, this function is robust enough to handle
 * nonpositive integers.
 */
int should_move_at_current_frame(const Game * const game, const int speed) {
    // Reasoning for rounding a double
    // Let FPS = 30 and speed = 16, if we perform integer division, we will get
    // one. This would be much faster than a speed of 16 would actually be as
    // ideally the object would be moved at every 1.875 frame. Therefore, it is
    // much better to update it at every other frame than at every frame. This
    // shows that the expected behavior is reached by rounding a precise
    // division rather than by truncating the quotient.
    if (speed == 0 || game->frame == 0) { // Play it safe with floating point errors
        return 0;
    } else {
        return game->frame % ((unsigned long) ((FPS / (double) abs(speed)) + 0.5)) == 0;
    }
}

void move_platform_horizontally(Game * const game, Platform * const platform) {
    Player * const player = game->player;
    if (should_move_at_current_frame(game, platform->speed_x)) {
        if (player->y == platform->y) { // Fail fast if the platform is not on the same line
            if (normalize(platform->speed_x) == 1) {
                if (player->x == platform->x + platform->width) {
                    Vector displacement;
                    displacement.x = 1;
                    displacement.y = 0;
                    shove_player(player, displacement);
                }
            } else if (normalize(platform->speed_x) == -1) {
                if (player->x == platform->x - 1) {
                    Vector displacement;
                    displacement.x = -1;
                    displacement.y = 0;
                    shove_player(player, displacement);
                }
            }
        } else if (is_over_platform(player->x, player->y, platform)) {  // If the player is over the platform
            Vector displacement;
            displacement.x = normalize(platform->speed_x);
            displacement.y = 0;
            shove_player(player, displacement);
        }
        platform->x += normalize(platform->speed_x);
    }
}

void move_platform_vertically(Game * const game, Platform * const platform) {
    Player * const player = game->player;
    if (should_move_at_current_frame(game, platform->speed_y)) {
            if (player->x >= platform->x && player->x < platform->x + platform->width) {
            if (normalize(platform->speed_y) == 1) {
                if (player->y == platform->y + 1) {
                    Vector displacement;
                    displacement.x = 0;
                    displacement.y = 1;
                    shove_player(player, displacement);
                }
            } else if (normalize(platform->speed_y) == -1) {
                if (player->y == platform->y - 1) {
                    Vector displacement;
                    displacement.x = 0;
                    displacement.y = -1;
                    shove_player(player, displacement);
                }
            }
        }
        platform->y += normalize(platform->speed_y);
    }
}

/**
 * Repositions a Platform in the vicinity of a BoundingBox.
 */
void reposition(Game * const game, Platform * const platform) {
    const BoundingBox * const box = game->box;
    if (platform->x > box->max_x) { // To the right of the box
        platform->x = 1 - platform->width;
        platform->y = random_integer(box->min_y, box->max_y);
    } else if (platform->x + platform->width < box->min_x) { // To the left of the box
        platform->x = box->max_x;
        platform->y = random_integer(box->min_y, box->max_y);
    } else if (platform->y < box->min_y) { // Above the box
        platform->x = random_integer(box->min_x, box->max_x);
        // Must work when the player is in the last line
        platform->y = box->max_y + 1; // Create it under the bounding box
        move_platform_vertically(game, platform); // Use the move function to keep the game in a valid state
    }
    // We don't have to deal with platforms below the box.
}

/**
 * Evaluates whether or not a Platform is completely outside of a BoundingBox.
 *
 * Returns 0 if the platform intersects the bounding box.
 * Returns 1 if the platform is to the left or to the right of the bounding box.
 * Returns 2 if the platform is above or below the bounding box.
 */
int is_out_of_bounding_box(Platform * const platform, const BoundingBox * const box) {
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

void update_platform(Game * const game, Platform * const platform) {
    move_platform_horizontally(game, platform);
    move_platform_vertically(game, platform);
    if (is_out_of_bounding_box(platform, game->box)) {
        reposition(game, platform);
    }
}

void update_platforms(Game * const game) {
    size_t i;
    for (i = 0; i < game->platform_count; i++) {
        update_platform(game, game->platforms + i);
    }
}

int is_valid_move(const int x, const int y, const Platform *platforms, const size_t platform_count) {
    size_t i;
    for (i = 0; i < platform_count; i++) {
        if (is_within_platform(x, y, platforms + i)) {
            return 0;
        }
    }
    return 1;
}

/**
 * Evaluates whether or not the Player is falling. Takes the physics field into account.
 */
int is_falling(const Player * const player, const Platform *platforms, const size_t platform_count) {
    if (!player->physics) {
        return 0;
    }
    size_t i;
    for (i = 0; i < platform_count; i++) {
        if (player->y == platforms[i].y - 1) {
            if (player->x >= platforms[i].x && player->x < platforms[i].x + platforms[i].width) {
                return 0;
            }
        }
    }
    return 1;
}

int is_touching_a_wall(const Player * const player, const BoundingBox * const box) {
    return (player->x < box->min_x || player->x > box->max_x) || (player->y < box->min_y || player->y > box->max_y);
}

int get_bounding_box_center_x(const BoundingBox * const box) {
    return box->min_x + (box->max_x - box->min_x + 1) / 2;
}

int get_bounding_box_center_y(const BoundingBox * const box) {
    return box->min_y + (box->max_y - box->min_y + 1) / 2;
}

void reposition_player(Player * const player, const BoundingBox * const box) {
    player->x = get_bounding_box_center_x(box);
    player->y = get_bounding_box_center_y(box);
}

/**
 * Conceives a bonus perk to the player.
 */
void conceive_bonus(Player * const player, Perk perk) {
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

void update_perk(Game * const game) {
    if (game->played_frames == game->perk_end_frame) {
        // Current Perk (if any) must end.
        game->perk = PERK_NONE;
    } else if (game->played_frames == game->perk_end_frame - PERK_DURATION_ON_SCREEN_IN_FRAMES + PERK_INTERVAL_IN_FRAMES) {
        // If the frame count since the current perk was created is equal to the perk interval, create a new Perk.
        game->perk = get_random_perk();
        game->perk_x = random_integer(game->box->min_x, game->box->max_x);
        game->perk_y = random_integer(game->box->min_y, game->box->max_y);
        game->perk_end_frame = game->played_frames + PERK_DURATION_ON_SCREEN_IN_FRAMES;
    }
}

/**
 * Moves the player according to the sign of its current speed if it can move in that direction.
 */
void move_player(Player * const player, const Platform *platforms, const size_t platform_count) {
    if (normalize(player->speed_x) == 1) {
        if (is_valid_move(player->x + 1, player->y, platforms, platform_count)) {
            player->x += 1;
        }
    } else if (normalize(player->speed_x) == -1) {
        if (is_valid_move(player->x - 1, player->y, platforms, platform_count)) {
            player->x -= 1;
        }
    }
}

int is_jumping(const Player * const player) {
    return player->remaining_jump_height > 0;
}

/**
 * Evaluates whether or not the player is standing on a platform.
 */
int is_standing_on_platform(const Player * const player, const Platform * platforms, const size_t platform_count) {
    size_t i;
    for (i = 0; i < platform_count; i++) {
        if (is_over_platform(player->x, player->y, platforms + i)) {
            return 1;
        }
    }
    return 0;
}

void process_jump(Player * const player, const Platform * platforms, const size_t platform_count) {
    if (is_standing_on_platform(player, platforms, platform_count)) {
        player->remaining_jump_height = PLAYER_JUMPING_HEIGHT;
        if (player->perk == PERK_POWER_SUPER_JUMP) {
            player->remaining_jump_height *= 2;
        }
    } else if (player->can_double_jump) {
        player->can_double_jump = 0;
        player->remaining_jump_height += PLAYER_JUMPING_HEIGHT / 2;
        if (player->perk == PERK_POWER_SUPER_JUMP) {
            player->remaining_jump_height *= 2;
        }
    }
}

void update_player(Game * const game, const Command command) {
    Player * const player = game->player;
    Platform *platforms = game->platforms;
    const size_t platform_count = game->platform_count;
    BoundingBox *box = game->box;
    if (command != COMMAND_NONE) {
        player->physics = 1;
    }
    if (player->physics) {
        game->played_frames++;
        // Check for expiration of the player's perk.
        if (player->perk != PERK_NONE) {
            if (game->played_frames == player->perk_end_frame) {
                player->perk = PERK_NONE;
            }
        }
        if (game->perk != PERK_NONE) {
            if (game->perk_x == player->x && game->perk_y == player->y) {
                // Copy the Perk to transfer it to the Player
                Perk perk = game->perk;

                // Remove the Perk from the screen
                game->perk = PERK_NONE;
                // Do not update game->perk_end_frame as it is used to
                // calculate when the next perk is going to be created

                // Attribute the Perk to the Player
                player->perk = perk;
                if (is_bonus_perk(perk)) {
                    conceive_bonus(player, perk);
                    player->perk_end_frame = game->played_frames; // The perk ended now.
                    // Could set it to the next frame so that the check above
                    // this part would removed it, but this seems more correct.
                    player->perk = PERK_NONE;
                } else {
                    player->perk_end_frame = game->played_frames + PERK_DURATION_ON_PLAYER_IN_FRAMES;
                }
            }
        }
    }
    // Update the player running state
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
        process_jump(player, platforms, platform_count);
    }
    // This ordering makes the player run horizontally before falling, which
    // seems the right thing to do to improve user experience.
    if (should_move_at_current_frame(game, player->speed_x)) {
        move_player(player, platforms, platform_count);
    }
    // After moving, if it even happened, simulate gravity.
    if (is_jumping(player)) {
        if (should_move_at_current_frame(game, PLAYER_JUMPING_SPEED)) {
            player->y--;
            player->remaining_jump_height--;
        }
    } else if (is_falling(player, platforms, platform_count)) {
        if (should_move_at_current_frame(game, PLAYER_FALLING_SPEED)) {
            player->y++;
        }
    }
    // Enable double jump if the player is standing over a platform.
    if (is_standing_on_platform(player, platforms, platform_count)) {
        player->can_double_jump = 1;
    }
    // Kill the player if it is touching a wall.
    if (is_touching_a_wall(player, box)) {
        player->lives--;
        reposition_player(player, box);
        // Unset physics collisions for the player.
        player->physics = 0;
        player->speed_x = 0;
        player->can_double_jump = 0;
        player->remaining_jump_height = 0;
    }
}
