#include "perk.hpp"
#include "logger.hpp"
#include "random.hpp"

Perk get_random_perk(RandomNumberGenerator &generator) {
  return static_cast<Perk>(generator.random_integer(0, PERK_COUNT - 1));
}

bool is_bonus_perk(Perk perk) {
  return perk == PERK_BONUS_EXTRA_POINTS || perk == PERK_BONUS_EXTRA_LIFE;
}

bool is_curse_perk(Perk perk) {
  return perk == PERK_CURSE_ACCELERATE_PLATFORMS || perk == PERK_CURSE_REVERSE_PLATFORMS;
}

std::string get_perk_name(Perk perk) {
  if (perk == PERK_POWER_INVINCIBILITY) {
    return "Invincibility";
  }
  if (perk == PERK_POWER_LEVITATION) {
    return "Levitation";
  }
  if (perk == PERK_POWER_FEATHER_FALL) {
    return "Feather Fall";
  }
  if (perk == PERK_POWER_SUPER_JUMP) {
    return "Super Jump";
  }
  if (perk == PERK_POWER_TIME_STOP) {
    return "Time Stop";
  }
  if (perk == PERK_CURSE_ACCELERATE_PLATFORMS) {
    return "Accelerate Platforms";
  }
  if (perk == PERK_CURSE_REVERSE_PLATFORMS) {
    return "Reverse Platforms";
  }
  if (perk == PERK_BONUS_EXTRA_POINTS) {
    return "Extra Points";
  }
  if (perk == PERK_BONUS_EXTRA_LIFE) {
    return "Extra Life";
  }
  log_message("Tried to get the name of an unnamed Perk!");
  return "Unnamed Perk";
}
