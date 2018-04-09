#ifndef PLAYER_H
#define PLAYER_H

#include "command.hpp"
#include "constants.hpp"
#include "graphics.hpp"
#include "perk.hpp"
#include "score.hpp"

class Player {
public:
  std::string name;

  CommandTable *table;

  Graphics graphics;

  int x;
  int y;
  int w;
  int h;
  int speed_x;
  int speed_y;

  /* Whether or not the player is being affected by physics. */
  bool physics;

  int can_double_jump;
  int remaining_jump_height;

  int lives;

  Score score;

  Perk perk;
  U64 perk_end_frame;

  Player(std::string name, CommandTable *table);

  void increment_score(Score amount);
  void decrement_score(Score amount);
  void increment_score_from_event(float rarity);

  inline bool is_jumping() const {
    return remaining_jump_height > 0;
  }
};

#endif
