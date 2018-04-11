#include "about.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "io.hpp"
#include "logger.hpp"
#include <cstring>

#define ABOUT_PAGE_PATH "assets/about/about.txt"

#define ABOUT_PAGE_BUFFER_SIZE 1024

/**
 * Presents information about the game to the player.
 */
Code info(const Settings &settings, Renderer &renderer, CommandTable *table) {
  char buffer[ABOUT_PAGE_BUFFER_SIZE];
  Code code = read_characters(ABOUT_PAGE_PATH, buffer, ABOUT_PAGE_BUFFER_SIZE);
  if (code != CODE_OK) {
    log_message("Failed to read the text.");
    return code;
  }
  renderer.print_long_text(buffer);
  return wait_for_input(settings, table);
}
