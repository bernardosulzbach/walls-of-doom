#ifndef WALLS_OF_DOOM_REPLAY_HPP
#define WALLS_OF_DOOM_REPLAY_HPP

#include "command.hpp"
#include <vector>

class Replay {
public:
  Replay(const CommandTableHistory &history);

private:
  CommandTableHistory history;
};

#endif
