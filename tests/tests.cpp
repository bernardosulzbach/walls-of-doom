#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"
#include "sources/data.hpp"
#include "sources/io.hpp"
#include "sources/logger.hpp"
#include "sources/numeric.hpp"
#include "sources/random.hpp"
#include "sources/sort.hpp"
#include "sources/text.hpp"
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sources/record_table.hpp>

#define SMALL_STRING_BUFFER_SIZE 64

#define LARGE_STRING_BUFFER_SIZE 2048

#define RESIZE_MEMORY_SIZE 4096

#define WRAP_TEST_SOURCE "assets/tests/wrap-test-source.txt"
#define WRAP_TEST_WIDTH_10 "assets/tests/wrap-test-width-10.txt"
#define WRAP_TEST_WIDTH_20 "assets/tests/wrap-test-width-20.txt"
#define WRAP_TEST_WIDTH_40 "assets/tests/wrap-test-width-40.txt"
#define WRAP_TEST_WIDTH_80 "assets/tests/wrap-test-width-80.txt"

TEST_CASE("normalize()") {
  REQUIRE(-1 == normalize(INT_MIN));
  REQUIRE(-1 == normalize(-1));
  REQUIRE(0 == normalize(0));
  REQUIRE(1 == normalize(1));
  REQUIRE(1 == normalize(INT_MAX));
}

TEST_CASE("get_random_perk() is well distributed") {
  const int maximum_allowed_deviation = 1 << 10;
  const int average = 1 << 16;
  const int test_count = average * PERK_COUNT;

  int counters[PERK_COUNT] = {0};

  int minimum = INT_MAX;
  int maximum = INT_MIN;
  int maximum_deviation = 0;
  double total = 0;

  int count;
  int i;

  seed_random();

  for (i = 0; i < test_count; i++) {
    counters[get_random_perk()]++;
  }
  /* Assess the distribution of the values. */
  for (i = 0; i < PERK_COUNT; i++) {
    count = counters[i];
    minimum = std::min(minimum, count);
    maximum = std::max(maximum, count);
    maximum_deviation = std::max(maximum_deviation, abs(count - average));
    total += count;
  }
  if (maximum_deviation > maximum_allowed_deviation) {
    FAIL("Maximum deviation is bigger than the allowed maximum.");
  }
}

TEST_CASE("get_full_path() checks for buffer overflow") {
  char buffer[MAXIMUM_PATH_SIZE];
  char big_filename[MAXIMUM_PATH_SIZE];
  Code code;
  memset(big_filename, 'A', MAXIMUM_PATH_SIZE);
  big_filename[MAXIMUM_PATH_SIZE - 1] = '\0';
  code = get_full_path(buffer, big_filename);
  REQUIRE(CODE_ERROR == code);
}

TEST_CASE("trim_string() works with empty strings") {
  const char *input = "";
  const char *expected = input;
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() works with already trimmed strings") {
  const char *input = "a b";
  const char *expected = input;
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims preceding spaces") {
  const char *input = "  a b";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims trailing spaces") {
  const char *input = "a b  ";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() properly trims space padded strings") {
  const char *input = "  a b  ";
  const char *expected = "a b";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("trim_string() works with strings of whitespaces") {
  const char *input = " \t \n ";
  const char *expected = "";
  char buffer[SMALL_STRING_BUFFER_SIZE];
  copy_string(buffer, input, SMALL_STRING_BUFFER_SIZE);
  trim_string(buffer);
  REQUIRE(string_equals(expected, buffer));
}

TEST_CASE("wrap_at_right_margin() with width 10") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_10, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 10);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 20") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_20, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 20);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 40") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_40, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 40);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with width 80") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  read_characters(WRAP_TEST_WIDTH_80, expected, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, 80);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("wrap_at_right_margin() with maximum width") {
  char expected[LARGE_STRING_BUFFER_SIZE];
  char actual[LARGE_STRING_BUFFER_SIZE];
  read_characters(WRAP_TEST_SOURCE, actual, LARGE_STRING_BUFFER_SIZE);
  REQUIRE(strlen(actual) != 0);
  /* This should NOT wrap the input, even on 32-bit implementations. */
  /* This is wrapping to (at least) 4,294,967,295 columns. */
  copy_string(expected, actual, LARGE_STRING_BUFFER_SIZE);
  wrap_at_right_margin(actual, SIZE_MAX);
  REQUIRE(string_equals(expected, actual));
}

TEST_CASE("read_integers()") {
  char filename[] = "test_read_integers_file.txt";
  const int expected_count = 1;
  const int expected_value = 65535;
  int actual_values[1];
  int actual_count;
  FILE *file;
  file = fopen(filename, "w+");
  if (file == nullptr) {
    FAIL("Failed to create helper test file.");
  }
  fprintf(file, "%d", expected_value);
  fclose(file);
  actual_count = read_integers(filename, actual_values, expected_count);
  REQUIRE(expected_count == actual_count);
  REQUIRE(expected_value == actual_values[0]);
  remove(filename);
}

TEST_CASE("bounding_box_equals()") {
  const int combinations = 1 << 4;
  BoundingBox box_i;
  BoundingBox box_j;
  int i;
  int j;
  for (i = 0; i < combinations; i++) {
    box_i.min_x = i >> 0 & 1;
    box_i.max_x = i >> 1 & 1;
    box_i.min_y = i >> 2 & 1;
    box_i.max_y = i >> 3 & 1;
    for (j = 0; j < combinations; j++) {
      box_j.min_x = j >> 0 & 1;
      box_j.max_x = j >> 1 & 1;
      box_j.min_y = j >> 2 & 1;
      box_j.max_y = j >> 3 & 1;
      if (i == j) {
        REQUIRE(box_i == box_j);
      } else {
        REQUIRE(box_i != box_j);
      }
    }
  }
}

TEST_CASE("generate_platforms() avoids multiple platforms on the same line") {
  const size_t platform_count = 128;
  const BoundingBox empty{0, 0, 0, 0};
  std::vector<int> y_counter(platform_count);
  for (size_t i = 0; i < platform_count; i++) {
    y_counter[i] = 0;
  }
  BoundingBox box;
  box.min_x = 0;
  box.min_y = 0;
  box.max_x = platform_count - 1;
  box.max_y = platform_count - 1;
  Settings settings(settings_filename);
  auto platforms = generate_platforms(settings, box, empty, platform_count, 1, 1);
  /* Each platform in platforms should have a different y coordinate. */
  for (size_t i = 0; i < platform_count; i++) {
    const auto y = platforms[i].y;
    /* Casting is safe because y is nonnegative at that point. */
    if (y >= 0 && static_cast<size_t>(y) < platform_count) {
      if (y_counter[y] != 0) {
        FAIL("Two of more platforms on the same line.");
      } else {
        y_counter[y]++;
      }
    } else {
      FAIL("Platform has invalid y coordinate.");
    }
  }
}

TEST_CASE("find_next_power_of_two() works for zero") {
  REQUIRE(find_next_power_of_two(0) == 1);
}

TEST_CASE("find_next_power_of_two() works for positive integers") {
  const unsigned long one = 1;
  unsigned long input;
  unsigned long expected;
  int i;
  /* Do not get to 31, because then the loop evaluates 1 << 32. */
  for (i = 0; i < 31; i++) {
    expected = one << (i + 1);
    /* Test the next power for the current power. */
    input = one << i;
    REQUIRE(expected == find_next_power_of_two(input));
    /* Test the next power for the next power minus one. */
    input = (one << (i + 1)) - 1;
    REQUIRE(expected == find_next_power_of_two(input));
  }
}

TEST_CASE("random_integer() respects the provided range") {
  const int min_min = -3;
  const int max_min = 0;
  const int min_max = 0;
  const int max_max = 4;
  int min;
  int max;
  int random_result;
  for (min = min_min; min < max_min; min++) {
    for (max = min_max; max < max_max; max++) {
      random_result = random_integer(min, max);
      REQUIRE((min <= random_result && random_result <= max));
    }
  }
}

TEST_CASE("random_integer() is evenly distributed") {
  /* Use primes for the boundaries as this may help show biases. */
  const int minimum = -227;
  const int maximum = 233;
  const int values = 227 + 1 + 233;
  const int expected_count = 1 << 8;
  const int minimum_allowed_count = 1 << 7;
  const int maximum_allowed_count = expected_count + minimum_allowed_count;
  int counters[227 + 1 + 233] = {0};
  int random_result;
  int i;
  for (i = 0; i < values * expected_count; i++) {
    random_result = random_integer(minimum, maximum);
    counters[random_result - minimum]++;
  }
  for (i = 0; i < values; i++) {
    if (counters[i] < minimum_allowed_count) {
      FAIL("Counter is below minimum allowed count.");
    } else if (counters[i] > maximum_allowed_count) {
      FAIL("Counter is above maximum allowed count.");
    }
  }
}

TEST_CASE("select_random_line_blindly() with one empty line") {
  const int tests = 1000;
  const std::vector<U8> array{0};
  for (int i = 0; i < tests; i++) {
    REQUIRE(0 == select_random_line_blindly(array));
  }
}

TEST_CASE("select_random_line_blindly() with two empty lines") {
  /* Should not overflow after a multiplication by 3. */
  const int tests = 10000;
  const int seven_sixteenths = 7 * tests / 16;
  const std::vector<U8> array{0, 0};
  int counters[2] = {0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[1] > seven_sixteenths);
}

TEST_CASE("select_random_line_blindly() with three empty lines") {
  const auto tests = 10000;
  const auto five_sixteenths = 5 * tests / 16;
  const std::vector<U8> array{0, 0, 0};
  int counters[3] = {0, 0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > five_sixteenths);
  REQUIRE(counters[1] > five_sixteenths);
  REQUIRE(counters[2] > five_sixteenths);
}

TEST_CASE("select_random_line_blindly() with one occupied line") {
  const auto tests = 10000;
  const std::vector<U8> array{1};
  for (int i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_blindly(array));
  }
}

TEST_CASE("select_random_line_blindly() with occupied middle line") {
  const auto tests = 10000;
  const auto seven_sixteenths = 7 * tests / 16;
  const std::vector<U8> array{0, 1, 0};
  int counters[3] = {0, 0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_blindly(array)] += 1;
  }
  REQUIRE(0 == counters[1]);
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[2] > seven_sixteenths);
}

TEST_CASE("select_random_line_awarely() with one empty line") {
  const int tests = 1000;
  const std::vector<U8> array{0};
  for (int i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_awarely(array));
  }
}

TEST_CASE("select_random_line_awarely() with two empty lines") {
  /* Should not overflow after a multiplication by 3. */
  const int tests = 10000;
  const int seven_sixteenths = 7 * tests / 16;
  const std::vector<U8> array{0, 0};
  int counters[2] = {0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array)] += 1;
  }
  /* Counters should be roughly the same. */
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[1] > seven_sixteenths);
}

TEST_CASE("select_random_line_awarely() with three empty lines") {
  const int tests = 10000;
  const std::vector<U8> array{0, 0, 0};
  int counters[3] = {0, 0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array)] += 1;
  }
  /* The middle line is the most distant one. */
  REQUIRE(counters[1] == tests);
}

TEST_CASE("select_random_line_awarely() with one occupied line") {
  const int tests = 1000;
  const std::vector<U8> array{1};
  for (int i = 0; i < tests; i++) {
    /* There is only one line to select, must select this one. */
    REQUIRE(0 == select_random_line_awarely(array));
  }
}

TEST_CASE("select_random_line_awarely() with occupied middle line") {
  const int tests = 10000;
  const int seven_sixteenths = 7 * tests / 16;
  const std::vector<U8> array{0, 1, 0};
  int counters[3] = {0, 0, 0};
  for (int i = 0; i < tests; i++) {
    counters[select_random_line_awarely(array)] += 1;
  }
  REQUIRE(0 == counters[1]);
  REQUIRE(counters[0] > seven_sixteenths);
  REQUIRE(counters[2] > seven_sixteenths);
}

TEST_CASE("parse_base16_digit_pair() works") {
  for (U32 i = 0; i < 256; i++) {
    std::stringstream stream;
    stream << std::hex;
    stream << std::setw(2);
    stream << std::setfill('0');
    stream << i;
    REQUIRE(i == parse_base16_digit_pair(stream.str()));
  }
}

TEST_CASE("color_pair_from_string() works") {
  ColorPair pair = color_pair_from_string("12345678,90ABCDEF");
  REQUIRE(pair.foreground.r == parse_base16_digit_pair("12"));
  REQUIRE(pair.foreground.g == parse_base16_digit_pair("34"));
  REQUIRE(pair.foreground.b == parse_base16_digit_pair("56"));
  REQUIRE(pair.foreground.a == parse_base16_digit_pair("78"));
  REQUIRE(pair.background.r == parse_base16_digit_pair("90"));
  REQUIRE(pair.background.g == parse_base16_digit_pair("AB"));
  REQUIRE(pair.background.b == parse_base16_digit_pair("CD"));
  REQUIRE(pair.background.a == parse_base16_digit_pair("EF"));
}

TEST_CASE("RecordTable maximum size is respected") {
  RecordTable table(2);
  const Record record_a("A", 2);
  const Record record_b("B", 1);
  const Record record_c("C", 1);
  const Record record_d("D", 2);
  const Record record_e("E", 3);
  REQUIRE(table.size() == 0);
  REQUIRE(std::vector<Record>(table.begin(), table.end()).empty());
  table.add_record(record_a);
  REQUIRE(table.size() == 1);
  REQUIRE(std::vector<Record>(table.begin(), table.end()) == std::vector<Record>{record_a});
  table.add_record(record_b);
  REQUIRE(table.size() == 2);
  REQUIRE(std::vector<Record>(table.begin(), table.end()) == std::vector<Record>{record_a, record_b});
  table.add_record(record_c);
  REQUIRE(table.size() == 2);
  REQUIRE(std::vector<Record>(table.begin(), table.end()) == std::vector<Record>{record_a, record_b});
  table.add_record(record_d);
  REQUIRE(table.size() == 2);
  REQUIRE(std::vector<Record>(table.begin(), table.end()) == std::vector<Record>{record_a, record_d});
  table.add_record(record_e);
  REQUIRE(table.size() == 2);
  REQUIRE(std::vector<Record>(table.begin(), table.end()) == std::vector<Record>{record_e, record_a});
}