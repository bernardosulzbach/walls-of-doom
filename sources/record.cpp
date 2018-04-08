#include "record.hpp"
#include "code.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "numeric.hpp"
#include "profiler.hpp"
#include "settings.hpp"
#include "sort.hpp"
#include "text.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

const U32 maximum_player_name_size = 64;

Record::Record(const std::string &desired_name, Score score) : name(desired_name), score(score) {
  if (name.size() > maximum_player_name_size) {
    log_message("Trimmed Record name.");
    name = name.substr(0, maximum_player_name_size);
  }
}

std::string Record::get_name() const {
  return name;
}

Score Record::get_score() const {
  return score;
}

bool Record::operator==(const Record &rhs) const {
  return name == rhs.name && score == rhs.score;
}

bool Record::operator!=(const Record &rhs) const {
  return !(rhs == *this);
}

bool Record::operator<(const Record &rhs) const {
  return score < rhs.score;
}

bool Record::operator>(const Record &rhs) const {
  return rhs < *this;
}

bool Record::operator<=(const Record &rhs) const {
  return !(rhs < *this);
}

bool Record::operator>=(const Record &rhs) const {
  return !(*this < rhs);
}
