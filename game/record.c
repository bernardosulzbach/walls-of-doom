#include "record.h"

#include "constants.h"
#include "data.h"
#include "io.h"
#include "logger.h"
#include "rest.h"
#include "sort.h"

#include "curses.h"

#include <stdio.h>
#include <string.h>

#define RECORD_ARRAY_SIZE 5

/*
 * The maximum number of records that will be displayed to the player.
 *
 * May be more than the maximum available.
 */
#define MAXIMUM_DISPLAYED_RECORDS 64

#define RECORD_TABLE_FILENAME "records.bin"

typedef struct RecordTable {
    size_t record_count;
    Record records[RECORD_ARRAY_SIZE];
} RecordTable;

/**
 * Creates a new Record from the provided data.
 *
 * If the name is too big to fit into the array, it is truncated to the maximum
 * possible size.
 */
Record make_record(const char * name, const int score) {
    Record record;
    
    /* Safely copy the provided name into the array. */
    strncpy(record.name, name, PLAYER_NAME_MAXIMUM_SIZE - 1);
    /* We must manually assure that the last char is a null character. */
    record.name[PLAYER_NAME_MAXIMUM_SIZE - 1] = '\0';

    record.score = score;
    
    return record;
}

int compare_records(const Record * const a, const Record * const b) {
    if (a->score < b->score) {
        return -1;
    } else if (a->score == b->score) {
        return 0;
    } else {
        return 1;
    };
}

int compare_void_record_pointers(const void *a, const void *b) {
    return compare_records((const Record *) a, (const Record *) b);
}

void populate_table_with_default_records(RecordTable * table) {
    static char *names[] = {"Adam", "Bree", "Cora", "Dave", "Elmo"};
    static int scores[] =  {    18,     14,     10,      8,      2};
    size_t i;
    size_t end = sizeof(scores) / sizeof(int);
    if (RECORD_ARRAY_SIZE < end) {
        end = RECORD_ARRAY_SIZE;
    }
    table->record_count = 0;
    for (i = 0; i < end; i++) {
        table->records[i] = make_record(names[i], scores[i]);
        table->record_count++;
    }
}

/**
 * Reads a RecordTable into the provided destination.
 *
 * If the existing table cannot be used a new one is created.
 */
void read_table(RecordTable * table) {
    int read_error = 0;
    if (file_exists(RECORD_TABLE_FILENAME)) {
        if (read_bytes(RECORD_TABLE_FILENAME, table, sizeof(RecordTable), 1)) {
            char message[512];
            sprintf(message, "Failed to read a RecordTable from %s", RECORD_TABLE_FILENAME);
            log_message(message);
            read_error = 1; /* Set the error flag to trigger the creation of a new table. */
        }
    } else {
        read_error = 1; /* Set the error flag to trigger the creation of a new table. */
    }
    if (read_error) {
        populate_table_with_default_records(table);
        log_message("Populated the table with the default records");
    }
}

void write_table(const RecordTable * const table) {
    write_bytes(RECORD_TABLE_FILENAME, table, sizeof(RecordTable), 1);
}

/**
 * Writes the specified Record to to the system.
 */
int save_record(const Record * const record) {
    /* Add 1 to the maximum size as we need to store the newest record. */
    RecordTable table;
    read_table(&table);

    /* If the table is full, overwrite the last record if this record is greater. */
    if (table.record_count == RECORD_ARRAY_SIZE) {
        if (compare_records(record, table.records + (table.record_count - 1)) > 0) {
            table.records[table.record_count - 1] = *record;
        } else {
            return -1;
        }
    } else {
        table.records[table.record_count] = *record;
        table.record_count++;
    }
    log_message("Added the record to the record table");

    /* Sort the table records. */
    insertion_sort((void *)table.records, table.record_count, sizeof(Record), compare_void_record_pointers);
    reverse((void *)table.records, table.record_count, sizeof(Record));
    log_message("Sorted the record table");

    /* Write the table to disk. */
    write_table(&table);
    /* Find the index of the provided record. */
    int record_index = table.record_count - 1;
    while (compare_records(record, table.records + record_index) != 0) {
        record_index--;
    }
    return record_index;
}

/**
 * Reads Records from the system into the specified destination. Reading at
 * most destination_size Records.
 *
 * Returns how many Records were actually read.
 */
size_t read_records(Record * destination, size_t destination_size) {
    RecordTable table;
    size_t i;
    size_t max_i;
    read_table(&table);
    max_i = table.record_count;
    if (destination_size < table.record_count) {
       max_i = destination_size; /* Do not copy more than the caller asked for. */
    }
    for (i = 0; i < max_i; i++) {
        destination[i] = table.records[i];
    }
    return i;
}

/**
 * Returns the number of characters used to represent the provided number on
 * base 10.
 */
int count_digits(int number) {
    char buffer[MAXIMUM_STRING_SIZE];
    sprintf(buffer, "%d", number);
    return strlen(buffer);
}

void record_to_string(const Record * const record, char *buffer, const int expected_width) {
    char padding_string[MAXIMUM_STRING_SIZE];
    memset(padding_string, '.', MAXIMUM_STRING_SIZE - 1);
    padding_string[MAXIMUM_STRING_SIZE - 1] = '\0';
    const char format[] = "%s%*.*s%d";
    int padding_length = expected_width - strlen(record->name) - count_digits(record->score);
    sprintf(buffer, format, record->name, padding_length, padding_length, padding_string, record->score);
}

/**
 * Loads and presents the top scores on the screen.
 */
void top_scores(void) {
    const int line_width = COLS - 6;
    Record records[MAXIMUM_DISPLAYED_RECORDS];
    int y = 2;
    const int line_count = LINES - 2 * y;
    size_t maximum_read_records = MAXIMUM_DISPLAYED_RECORDS;
    size_t actually_read_records = read_records(records, maximum_read_records);
    char line[COLS];
    size_t i;
    if (COLS < 16) {
        return;
    }
    if (line_count > MAXIMUM_DISPLAYED_RECORDS) {
        maximum_read_records = MAXIMUM_DISPLAYED_RECORDS;
    } else {
        maximum_read_records = line_count;
    }
    clear();
    for (i = 0; i < actually_read_records; i++) {
        record_to_string(records + i, line, line_width);
        print_centered(y + i, line);
    }
    refresh();
    rest_for_seconds(3);
}

