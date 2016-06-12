#include "data.h"

#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Assesses whether or not a file with the provided filename exists.
 */
int file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

typedef enum Operation {
    READ,
    WRITE
} Operation;

void log_access(Operation operation, const size_t byte_count, const char *filename) {
    char message[512];
    if (operation == READ) {
        sprintf(message, "Reading %lu bytes from %s", (unsigned long) byte_count, filename); 
    } else {
        sprintf(message, "Writing %lu bytes to %s", (unsigned long) byte_count, filename); 
    }
    log_message(message);
}

/**
 * Writes bytes to the indicated file from the provided source.
 *
 * Returns 0 in case of success.
 */
int write_bytes(const char *filename, const void *source, const size_t size, const size_t count) {
    log_access(WRITE, size * count, filename);
    FILE *file = fopen(filename, "wb");
    fwrite(source, size, count, file);
    fclose(file);
    return 0;
}

/**
 * Reads bytes from the indicated file to the provided destination.
 *
 * Returns 0 in case of success.
 */
int read_bytes(const char *filename, void *destination, const size_t size, const size_t count) {
    log_access(READ, size * count, filename);
    if (file_exists(filename)) {
        FILE *file = fopen(filename, "rb");
        const size_t read_items = fread(destination, size, count, file);
        fclose(file);
        if (read_items != count) {
            char message[512];
            sprintf(message, "Expected to read %lu but actually read %lu", (unsigned long) count, (unsigned long) read_items);
            log_message(message);
            return 2;
        }
        return 0;
    } else {
        return 1;
    }
}

/**
 * Reads characters from the indicated file into the provided character string.
 *
 * Returns 0 in case of success.
 */
int read_characters(const char * const filename, char *destination, const size_t destination_size) {
    log_access(READ, destination_size, filename);
    if (file_exists(filename)) {
        FILE *file = fopen(filename, "r");
        size_t copied = 0;
        int c; /* Must be an integer because it may be EOF */
        /* Check copied + 1 against destination size because we need a null character at the end. */
        while (copied + 1 < destination_size && (c = fgetc(file)) != EOF) {
            destination[copied] = (char)c;
            copied++;
        }
        /* Done copying, place a null character if we can. */
        if (destination_size > 0) { /* Provided size may be 0. */
            destination[copied] = '\0';
        }
        fclose(file);
        return 0;
    } else {
        return 1;
    }
}

/**
 * Reads integers from the indicated file into the provided array.
 *
 * Returns the number of integers read.
 */
size_t read_integers(const char * const filename, int *integer_array, const size_t integer_array_size) {
    FILE *file;
    size_t next_index = 0;
    file = fopen(filename, "r");
    if (file) {
        while (next_index < integer_array_size && fscanf(file, "%d", &integer_array[next_index]) != EOF) {
            next_index++;
        }
        fclose(file);
    }
    return next_index; /* number of elements == index of next element */
}

