#include "platform.hpp"
#include "data.hpp"
#include "logger.hpp"
#include "physics.hpp"
#include <cstring>

PlatformRules::PlatformRules(const BoundingBox &box, const BoundingBox &avoidance, S32 width, S32 height, U64 count)
    : box(box), avoidance(avoidance), width(width), height(height), count(count) {
}

// TODO: test that this always respects avoidance.
// TODO: test that this throws on avoidance not contained in boundaries.
std::vector<Platform> generate_platforms(const Context &context, PlatformRules rules) {
  const S32 min_width = context.settings.get_platform_min_width() * rules.width;
  const S32 max_width = context.settings.get_platform_max_width() * rules.width;
  const S32 min_speed = context.settings.get_platform_min_speed();
  const S32 max_speed = context.settings.get_platform_max_speed();
  const auto lines = static_cast<U32>((rules.box.max_y - rules.box.min_y + 1) / rules.height);
  std::vector<U8> density(lines);
  for (S32 y = rules.avoidance.min_y; y < rules.avoidance.max_y; y++) {
    density[y / rules.height] = 1;
  }
  std::vector<Platform> platforms;
  for (U64 i = 0; i < rules.count; i++) {
    platforms.emplace_back();
    Platform &platform = platforms.back();
    platform.h = rules.height;
    platform.w = context.random_number_generator.random_integer(min_width, max_width);
    /* Subtract two to remove the borders. */
    /* Subtract one after this to prevent platform being after the screen. */
    platform.x = context.random_number_generator.random_integer(0, bounding_box_width(&rules.box)) + rules.box.min_x;
    const auto random_y = select_random_line_awarely(context.random_number_generator, density);
    density[random_y]++;
    platform.y = random_y * rules.height + rules.box.min_y;
    platform.speed = 0;
    const auto speed = context.random_number_generator.random_integer(min_speed, max_speed);
    /* Make about half the platforms go left and about half go right. */
    /* Make sure that the position is OK to trigger repositioning. */
    if (context.random_number_generator.random_integer(0, 1) != 0) {
      platform.speed = speed;
    } else {
      platform.speed = -speed;
    }
    platform.rarity = context.random_number_generator.random_integer(0, 4) / 4.0f;
  }
  return platforms;
}

bool Platform::operator==(const Platform &rhs) const {
  return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h && speed == rhs.speed && rarity == rhs.rarity;
}

bool Platform::operator!=(const Platform &rhs) const {
  return !(rhs == *this);
}
