#ifndef PHYSICS_H
#define PHYSICS_H

#include "box.hpp"
#include "command.hpp"
#include "game.hpp"
#include "platform.hpp"
#include "player.hpp"
#include <cstdlib>

static const int EXTRA_POINTS_AMOUNT = 1000;

/**
 * From an array of lines occupancy states, selects at random an empty line.
 *
 * If no such line exists, returns a random line.
 *
 * This algorithm is O(n) with respect to the number of lines.
 */
int select_random_line_blindly(RandomNumberGenerator &generator, const std::vector<unsigned char> &lines);

/**
 * Selects at random one of the lines which are the furthest away from any other occupied line.
 *
 * This algorithm is O(n) with respect to the number of lines.
 */
int select_random_line_awarely(RandomNumberGenerator &generator, const std::vector<unsigned char> &lines);

void update_platforms(Game *const game);

void update_perk(Game &game);

void update_player(Game *game, Player *player);

void reposition_player(Game *const game);

/**
 * Conceives a bonus perk to the player.
 */
void conceive_bonus(Player *const player, const Perk perk);

#endif
