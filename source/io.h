/**
 * Copyright (c) 2016, Bernardo Sulzbach and Leonardo Ferrazza
 * All rights reserved.
 *
 * See LICENSE.txt for more details.
 */

#ifndef IO_H
#define IO_H

#include <curses.h>

#include "platform.h"

#define DEFAULT_LINE "█"
#define DEFAULT_L_CORNER "█"
#define DEFAULT_R_CORNER "█"

#define HEAVY_LINE "━"
#define HEAVY_L_CORNER "┏"
#define HEAVY_R_CORNER "┓"

#define LIGHT_LINE "─"
#define LIGHT_L_CORNER "┌"
#define LIGHT_R_CORNER "┐"

/**
 * Prints the provided string on the screen starting at (x, y).
 */
void print(const int x, const int y, const char *string) {
    mvprintw(y, x, string);
}

typedef enum PlatformStyle {
    PLATFORM_DEFAULT,
    PLATFORM_HEAVY,
    PLATFORM_LIGHT
} PlatformStyle;

void print_platform(const Platform * const platform, const PlatformStyle style) {
    char *line = DEFAULT_LINE;
    char *begin = DEFAULT_L_CORNER;
    char *end = DEFAULT_R_CORNER;
    if (style == PLATFORM_HEAVY) {
        line = HEAVY_LINE;
        begin = HEAVY_L_CORNER;
        end = HEAVY_R_CORNER;
    } else if (style == PLATFORM_LIGHT) {
        line = LIGHT_LINE;
        begin = LIGHT_L_CORNER;
        end = LIGHT_R_CORNER;
    }
    unsigned int i;
    for (i = 0; i < platform->width; i++) {
        if (i == 0) {
            print(platform->x + i, platform->y, begin);
        } else if (i == platform->width - 1) {
            print(platform->x + i, platform->y, line);
        } else {
            print(platform->x + i, platform->y, end);
        }
    }
}

#endif
