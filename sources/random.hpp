#ifndef RANDOM_H
#define RANDOM_H

#include "integers.hpp"
#include <string>

extern const U64 default_random_number_generator_seed;

class RandomNumberGenerator {
public:
  explicit RandomNumberGenerator(U64 seed);

  /**
   * Returns the next power of two bigger than the provided number.
   */
  U64 find_next_power_of_two(U64 number);

  /**
   * Returns a random number in the range [minimum, maximum].
   */
  S32 random_integer(S32 minimum, S32 maximum);

  std::string get_user_name();

  U64 get_calls() const;

  U64 get_bad_calls() const;

private:
  U64 calls = 0;
  U64 bad_calls = 0;

  U64 x;
  U64 y = 0;
  U64 z = 0;
  U64 w = 0;

  U64 xorshift128();

  std::string random_word(const std::string &filename);
  std::string get_stored_name();
  std::string get_random_name();
};

#endif
