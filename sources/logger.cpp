#include "logger.hpp"
#include "clock.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "score.hpp"
#include "settings.hpp"
#include "version.hpp"
#include <cstdio>
#include <ctime>
#include <fstream>

#define LOGGER_VERSION_MESSAGE "Version is " WALLS_OF_DOOM_VERSION "."

#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"
#define TIMESTAMP_BUFFER_SIZE 64

#define LOG_FILE_NAME "log.txt"

#define SCORE_FILE_NAME "score.txt"

#define LOG_MESSAGE_SIZE (TIMESTAMP_BUFFER_SIZE + 256)

/**
 * Initializes the logger. Should only be called once.
 */
void initialize_logger() {
  /* Currently, the logger does not need initialization. */
  log_message("Initialized the logger.");
  log_message(LOGGER_VERSION_MESSAGE);
}

/**
 * Terminates the logger. Should only be called once.
 */
void finalize_logger() {
  /* Currently, the logger does not need termination. */
  log_message("Finalized the logger.");
}

/**
 * Writes a timestamp to the provided buffer.
 *
 * TIMESTAMP_BUFFER_SIZE is the minimum size for the timestamp buffer.
 *
 * This function does not rely on dynamic memory allocation.
 */
static void write_timestamp(char *buffer, const size_t buffer_size) {
  time_t current_time;
  struct tm *time_info;
  time(&current_time);
  time_info = localtime(&current_time);
  strftime(buffer, buffer_size, TIMESTAMP_FORMAT, time_info);
}

static void append_to_file(const std::string &path, const char *string) {
  std::ofstream stream(path, std::ios::app);
  stream << string << '\n';
}

/**
 * Logs the provided message to the current log file.
 */
void log_message(const std::string &message) {
  /*
   * Note that this function CANNOT use resize_memory because resize_memory calls this function.
   *
   * Using resize_memory here may cause a deadlock.
   */
  char stamp[TIMESTAMP_BUFFER_SIZE];
  char string[LOG_MESSAGE_SIZE];
  /* get_full_path does not use dynamic memory allocation. */
  const auto path = get_full_path(LOG_FILE_NAME);
  /* write_timestamp does not use dynamic memory allocation. */
  write_timestamp(stamp, TIMESTAMP_BUFFER_SIZE);
  sprintf(string, "[%s] %s", stamp, message.c_str());
  /* append_to_file does not use dynamic memory allocation. */
  append_to_file(path, string);
}

void log_player_score(const Settings &settings, const U64 frame, const Score score) {
  char string[LOG_MESSAGE_SIZE];
  if (settings.is_logging_player_score() != 0) {
    sprintf(string, "%ld,%ld", frame, score);
    append_to_file(get_full_path(SCORE_FILE_NAME), string);
  }
}
