#ifndef REST_H
#define REST_H

#include <time.h>
#include <stdint.h>

/**
 * Rests for the specified number of seconds.
 */
void rest_for_seconds(uint64_t seconds);

/**
 * Rests for the specified number of microseconds.
 */
void rest_for_microseconds(uint64_t microseconds);

/**
 * Rests for the reciprocal of the provided argument seconds.
 */
void rest_for_second_fraction(int fps);

#endif
