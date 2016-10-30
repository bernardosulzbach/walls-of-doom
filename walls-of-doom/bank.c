#include "investment.h"
#include "bank.h"
#include "random.h"
#include "settings.h"
#include "game.h"

static long calculate_playable_area(Game const *const game) {
  long area = 0;
  size_t i;
  for (i = 0; i != game->platform_count; ++i) {
    area += game->platforms[i].width;
  }
  return area;
}

/**
 * Returns a normalized value (from 0 to 1) indicating the game difficulty.
 */
static double get_difficulty(Game const *const game) {
  const double playable_area = (double)bounding_box_area(game->box);
  const double platform_area = (double)calculate_playable_area(game);
  return (playable_area - platform_area) / playable_area;
}

int collect_investment(Game const *const game, const Investment investment) {
  const int maximum_return = get_investment_maximum_factor();
  const int minimum_return = get_investment_minimum_factor();
  const int normalized_max = (int)(get_difficulty(game) * maximum_return);
  const int random_factor = random_integer(minimum_return, normalized_max);
  return random_factor * investment.amount / 100;
}
