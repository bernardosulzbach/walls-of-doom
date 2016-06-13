#include "about.h"

#include "data.h"
#include "io.h"
#include "rest.h"

#include <string.h>

#define ABOUT_PAGE_PATH "assets/about.txt"
#define ABOUT_PAGE_BUFFER_SIZE 1024

/**
 * Presents information about the game to the player.
 */
void info(void) {
    char buffer[ABOUT_PAGE_BUFFER_SIZE];
    int read_error = read_characters(ABOUT_PAGE_PATH, buffer, ABOUT_PAGE_BUFFER_SIZE);
    if (read_error) {
        log_message("Failed to read the text");
    } else {
        print_long_text(buffer);
        rest_for_seconds(6);
    }
}
