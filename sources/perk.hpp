#ifndef PERK_H
#define PERK_H

#include "random.hpp"
#include <string>

enum Perk {
  PERK_POWER_INVINCIBILITY,
  PERK_POWER_LEVITATION,
  PERK_POWER_FEATHER_FALL,
  PERK_POWER_SUPER_JUMP,
  PERK_POWER_TIME_STOP,
  PERK_CURSE_ACCELERATE_PLATFORMS,
  PERK_CURSE_REVERSE_PLATFORMS,
  PERK_BONUS_EXTRA_POINTS,
  PERK_BONUS_EXTRA_LIFE,
  PERK_COUNT,
  PERK_NONE
};

Perk get_random_perk(RandomNumberGenerator &generator);

bool is_bonus_perk(Perk perk);

bool is_curse_perk(Perk perk);

std::string get_perk_name(Perk perk);

#endif
