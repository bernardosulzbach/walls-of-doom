#ifndef DATA_H
#define DATA_H

#include "code.hpp"
#include "integers.hpp"
#include <cstdio>
#include <string>

/**
 * The maximum size a file path can have, including the NUL terminator.
 *
 * This is a project-specific limit.
 */
#define MAXIMUM_PATH_SIZE 512

/**
 * Just in case a platform may have longer path separators, use a constant.
 */
#define SEPARATOR_SIZE 1

/**
 * Assesses whether or not a file with the provided filename exists.
 */
bool file_exists(const std::string &filename);

/**
 * Returns the number of lines in a file.
 *
 * This is the count of occurrences of '\n'.
 */
U64 file_line_count(const std::string &filename);

std::string get_full_path(const std::string &filename);

/**
 * Writes to buffer the full path for a file created by Walls of Doom.
 *
 * If one needs to access the log.txt file, one should use
 *
 *   char path[MAXIMUM_PATH_SIZE];
 *   get_full_path(path, "log.txt");
 *
 * This is the correct way to access mutable files in any platform.
 */
Code get_full_path(char *buffer, const char *filename);

/**
 * Writes bytes to the indicated file from the provided source.
 */
Code write_bytes(const char *filename, const void *source, const size_t size, const size_t count);

Code write_string(const char *filename, const std::string &string);

/**
 * Reads bytes from the indicated file to the provided destination.
 */
Code read_bytes(const char *filename, void *destination, const size_t size, const size_t count);

/**
 * Reads characters from the indicated file into the provided character string.
 */
Code read_characters(const char *const filename, char *destination, const size_t destination_size);

Code read_characters(const char *const filename, std::string &destination);

/**
 * Reads integers from the indicated file into the provided array.
 *
 * Returns the number of integers read.
 */
size_t read_integers(const char *const filename, int *integer_array, const size_t integer_array_size);

#endif
