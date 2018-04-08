#include "color.hpp"
#include "text.hpp"
#include <SDL.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

/* These colors work as fallback if settings fails. */
Color COLOR_DEFAULT_FOREGROUND = {192, 192, 192, 255};
Color COLOR_DEFAULT_BACKGROUND = {32, 32, 32, 255};

ColorPair COLOR_PAIR_DEFAULT = {{192, 192, 192, 255}, {32, 32, 32, 255}};
ColorPair COLOR_PAIR_PERK = {{192, 192, 192, 255}, {32, 32, 32, 255}};
ColorPair COLOR_PAIR_PLAYER = {{192, 192, 192, 255}, {32, 32, 32, 255}};

ColorPair COLOR_PAIR_TOP_BAR = {{192, 192, 192, 255}, {32, 32, 32, 255}};
ColorPair COLOR_PAIR_BOTTOM_BAR = {{192, 192, 192, 255}, {32, 32, 32, 255}};

ColorPair COLOR_PAIR_PLATFORM_A = {{192, 192, 192, 255}, {32, 32, 32, 255}};
ColorPair COLOR_PAIR_PLATFORM_B = {{192, 192, 192, 255}, {32, 32, 32, 255}};

static void assert_equals(size_t actual, size_t expected) {
  if (expected != actual) {
    const auto expected_string = std::to_string(expected);
    const auto actual_string = std::to_string(actual);
    throw std::logic_error("Bad size, expected " + expected_string + " but got " + actual_string + ".");
  }
}

static U8 parse_base16_digit(char digit) {
  if (digit >= '0' && digit <= '9') {
    return digit - '0';
  }
  if (digit >= 'A' && digit <= 'F') {
    return 10 + (digit - 'A');
  } else if (digit >= 'a' && digit <= 'f') {
    return 10 + (digit - 'a');
  }
  throw std::logic_error("Conversion failed from '" + std::to_string(static_cast<S32>(digit)) + "'.");
}

U8 parse_base16_digit_pair(std::string string) {
  assert_equals(string.size(), 2);
  return parse_base16_digit(string[0]) * 16 + parse_base16_digit(string[1]);
}

static Color color_from_string(const std::string &string) {
  assert_equals(string.size(), 8);
  const auto r = parse_base16_digit_pair(string.substr(0, 2));
  const auto g = parse_base16_digit_pair(string.substr(2, 2));
  const auto b = parse_base16_digit_pair(string.substr(4, 2));
  const auto a = parse_base16_digit_pair(string.substr(6, 2));
  return {r, g, b, a};
}

ColorPair color_pair_from_string(const std::string &string) {
  assert_equals(string.size(), 8 + 1 + 8);
  const auto foreground = color_from_string(string.substr(0, 8));
  const auto background = color_from_string(string.substr(8 + 1, 8));
  return {foreground, background};
}
