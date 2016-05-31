#include "logger.h"
#include "physics.h"
#include "random.h"
#include "vector.h"

#include <math.h>
#include <stdio.h>

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
 * Evaluates whether or not a position is within a Platform.
 */
int is_within_platform(Vector position, const Platform * const platform) {
    if (position.y == platform->position.y) {
        if (position.x >= platform->position.x && position.x < platform->position.x + platform->width) {
            return 1;
        }
    }
    return 0;
}

/**
 * Evaluates whether or not a position is over a Platform.
 */
int is_over_platform(Vector position, const Platform * const platform) {
    position.y += 1.0;
    return is_within_platform(position, platform);
}

/**
 * Attempts to force the Player to move according to the provided displacement.
 *
 * If the player does not have physics enabled, this is a no-op.
 */
void shove_player(Player * const player, const Vector displacement) {
    if (player->physics) {
        player->position = vector_add(player->position, displacement);
    }
}

void move_platform_horizontally(Player * const player, Platform * const platform, const int direction) {
    log_if_not_normalized(direction);
    if (player->position.y == platform->position.y) { // Fail fast if the platform is not on the same line
        if (direction == 1) {
            if (player->position.x == platform->position.x + platform->width) {
                Vector displacement;
                displacement.x = 1;
                displacement.y = 0;
                shove_player(player, displacement);
            }
        } else if (direction == -1) {
            if (player->position.x == platform->position.x - 1) {
                Vector displacement;
                displacement.x = -1;
                displacement.y = 0;
                shove_player(player, displacement);
            }
        }
    } else if (is_over_platform(player->position, platform)) { // If the player is over the platform
        Vector displacement;
        displacement.y = 0;
        if (direction == 1) {
            displacement.x = 1;
        } else if (direction == -1) {
            displacement.x = -1;
        } else {
            displacement.x = 0;
        }
        shove_player(player, displacement);
    }
    platform->position.x += direction;
}

void move_platform_vertically(Player * const player, Platform * const platform, const int direction) {
    log_if_not_normalized(direction);
    if (player->position.x >= platform->position.x && player->position.x < platform->position.x + platform->width) {
        if (direction == 1) {
            if (player->position.y == platform->position.y + 1) {
                Vector displacement;
                displacement.x = 0;
                displacement.y = 1;
                shove_player(player, displacement);
            }
        } else if (direction == -1) {
            if (player->position.y == platform->position.y - 1) {
                Vector displacement;
                displacement.x = 0;
                displacement.y = -1;
                shove_player(player, displacement);
            }
        }
    }
    platform->position.y += direction;
}

/**
 * Repositions a Platform in the vicinity of a BoundingBox.
 */
void reposition(Player * const player, Platform * const platform, const BoundingBox * const box) {
    if (platform->position.x > box->max_x) { // To the right of the box
        platform->position.x = 1 - platform->width;
        platform->position.y = random_integer(box->min_y, box->max_y);
    } else if (platform->position.x + platform->width < box->min_x) { // To the left of the box
        platform->position.x = box->max_x;
        platform->position.y = random_integer(box->min_y, box->max_y);
    } else if (platform->position.y < box->min_y) { // Above the box
        platform->position.x = random_integer(box->min_x, box->max_x);
        // Must work when the player is in the last line
        platform->position.y = box->max_y + 1; // Create it under the bounding box
        move_platform_vertically(player, platform, -1); // Use the move function to keep the game in a valid state
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
    const int min_x = platform->position.x;
    const int max_x = platform->position.x + platform->width;
    if (max_x < box->min_x || min_x > box->max_x) {
        return 1;
    } else if (platform->position.y < box->min_y || platform->position.y > box->max_y) {
        return 2;
    } else {
        return 0;
    }
}

void update_platform(Player * const player, Platform * const platform, const BoundingBox * const box) {
    int i;
    for (i = 0; i < fabs(platform->velocity.x); i++) {
        move_platform_horizontally(player, platform, normalize(platform->velocity.x));
    }
    for (i = 0; i < fabs(platform->velocity.y); i++) {
        move_platform_vertically(player, platform, normalize(platform->velocity.y));
    }
    if (is_out_of_bounding_box(platform, box)) {
        reposition(player, platform, box);
    }
}

void update_platforms(Game * const game) {
    size_t i;
    for (i = 0; i < game->platform_count; i++) {
        update_platform(game->player, game->platforms + i, game->box);
    }
}

int is_valid_move(const Vector destination, const Platform *platforms, const size_t platform_count) {
    size_t i;
    for (i = 0; i < platform_count; i++) {
        if (is_within_platform(destination, platforms + i)) {
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
        if (player->position.y == platforms[i].position.y - 1) {
            if (player->position.x >= platforms[i].position.x && player->position.x < platforms[i].position.x + platforms[i].width) {
                return 0;
            }
        }
    }
    return 1;
}

int is_touching_a_wall(const Player * const player, const BoundingBox * const box) {
    return (player->position.x < box->min_x || player->position.x > box->max_x) || (player->position.y < box->min_y || player->position.y > box->max_y);
}

int get_bounding_box_center_x(const BoundingBox * const box) {
    return box->min_x + (box->max_x - box->min_x + 1) / 2;
}

int get_bounding_box_center_y(const BoundingBox * const box) {
    return box->min_y + (box->max_y - box->min_y + 1) / 2;
}

void reposition_player(Player * const player, const BoundingBox * const box) {
    player->position.x = get_bounding_box_center_x(box);
    player->position.y = get_bounding_box_center_y(box);
}

void update_perk(Game * const game) {
    if (game->played_frames == game->perk_end_frame) { // Current Perk must end
        game->perk = PERK_NONE;
    } else if (game->played_frames == game->perk_end_frame + GAME_PERK_INTERVAL_IN_FRAMES) {
        game->perk = get_random_perk();
        Vector position;
        position.x = random_integer(game->box->min_x, game->box->max_x);
        position.y = random_integer(game->box->min_y, game->box->max_y);
        game->perk_position = position;
        game->perk_end_frame = game->played_frames + GAME_PERK_DURATION_IN_FRAMES;
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
    }
    if (command == COMMAND_LEFT) {
        Vector destination = player->position;
        destination.x -= 1.0;
        if (is_valid_move(destination, platforms, platform_count)) {
            player->position.x -= 1.0;
        }
    } else if (command == COMMAND_RIGHT) {
        Vector destination = player->position;
        destination.x += 1.0;
        if (is_valid_move(destination, platforms, platform_count)) {
            player->position.x += 1.0;
        }
    }
    // After moving, if it even happened, simulate gravity
    if (is_falling(player, platforms, platform_count)) {
        player->position.y++;
    }
    if (is_touching_a_wall(player, box)) {
        player->lives--;
        reposition_player(player, box);
        // Unset physics collisions for the player.
        player->physics = 0;
    }
}
