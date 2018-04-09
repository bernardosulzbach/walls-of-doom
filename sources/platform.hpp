#ifndef PLATFORM_H
#define PLATFORM_H

#include "box.hpp"
#include "integers.hpp"
#include "settings.hpp"
#include <vector>

class Platform {
public:
  int x{};
  int y{};
  int w{};
  int h{};
  int speed{};
  float rarity = 0.0f;

  bool operator==(const Platform &rhs) const;
  bool operator!=(const Platform &rhs) const;
};

std::vector<Platform> generate_platforms(const Settings &settings, BoundingBox box, BoundingBox avoidance, U64 count);

#endif
