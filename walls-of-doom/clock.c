#include <SDL.h>

#include <stdint.h>

/**
 * Returns a number of milliseconds.
 *
 * This function should be used to measure computation times.
 */
uint32_t get_milliseconds(void) { return SDL_GetTicks(); }
