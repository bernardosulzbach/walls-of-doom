#ifndef PLATFORM_H
#define PLATFORM_H

#include "box.hpp"
#include "context.hpp"
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

class PlatformRules {
public:
  const BoundingBox box;
  const BoundingBox avoidance;
  const S32 width;
  const S32 height;
  const U64 count;

  PlatformRules(const BoundingBox &box, const BoundingBox &avoidance, S32 width, S32 height, U64 count);
};

std::vector<Platform> generate_platforms(const Context &context, PlatformRules rules);

#endif
