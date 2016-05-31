#ifndef PHYSICS_H
#define PHYSICS_H

#include "box.h"
#include "command.h"
#include "game.h"
#include "platform.h"
#include "player.h"
#include "stdlib.h"

int bounding_box_equals(const BoundingBox * const a, const BoundingBox * const b);

/**
 * Repositions a Platform in the vicinity of a BoundingBox.
 */
void reposition(Player * const player, Platform * const platform, const BoundingBox * const box);

/**
 * Evaluates whether or not a Platform is completely outside of a BoundingBox.
 *
 * Returns 0 if the platform intersects the bounding box.
 * Returns 1 if the platform is to the left or to the right of the bounding box.
 * Returns 2 if the platform is above or below the bounding box.
 */
int is_out_of_bounding_box(Platform * const platform, const BoundingBox * const box);

void update_platform(Player * const player, Platform * const platform, const BoundingBox * const box);

void update_platforms(Game * const game);

void update_perk(Game * const game);

void update_player(Game * const game, const Command command);

#endif
