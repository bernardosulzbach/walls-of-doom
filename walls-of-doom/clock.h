#ifndef CLOCK_H
#define CLOCK_H

typedef unsigned long Milliseconds;

/**
 * Returns a number of milliseconds.
 *
 * This function should be used to measure computation times.
 */
Milliseconds get_milliseconds(void);

/**
 * Sleeps for the specified number of milliseconds or more.
 */
void sleep_milliseconds(Milliseconds amount);

#endif
