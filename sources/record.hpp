#ifndef RECORD_H
#define RECORD_H

#include "code.hpp"
#include "constants.hpp"
#include "player.hpp"
#include <SDL.h>
#include <cstdlib>
#include <string>

extern const U32 maximum_player_name_size;

/**
 * A Record is a structure that stores the outcome of a game.
 */
class Record {
public:
  Record(const std::string &name, Score score);

  std::string get_name() const;

  Score get_score() const;

  bool operator==(const Record &rhs) const;

  bool operator!=(const Record &rhs) const;

  bool operator<(const Record &rhs) const;

  bool operator>(const Record &rhs) const;

  bool operator<=(const Record &rhs) const;

  bool operator>=(const Record &rhs) const;

private:
  std::string name;

  Score score;
};

#endif
