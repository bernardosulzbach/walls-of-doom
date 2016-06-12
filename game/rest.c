#include "rest.h"

#include "logger.h"

#include <unistd.h>

#define MICROSECONDS_IN_ONE_SECOND 1000000L

/**
 * Rests for the specified number of seconds.
 */
void rest_for_seconds(uint64_t seconds) {
    rest_for_microseconds(seconds * MICROSECONDS_IN_ONE_SECOND);
}

/**
 * Rests for the specified number of microseconds.
 */
void rest_for_microseconds(uint64_t microseconds) {
    usleep(microseconds);
}

/**
 * Rests for the reciprocal of the provided argument seconds.
 */
void rest_for_second_fraction(int fps) {
    if (fps < 1) {
        return;
    }
    rest_for_microseconds(MICROSECONDS_IN_ONE_SECOND / fps);
}
