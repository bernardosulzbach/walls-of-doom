#include "unity.h"

#include "data.h"
#include "io.h"
#include "logger.h"
#include "math.h"
#include "random.h"
#include "rest.h"
#include "sort.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int compare_unsigned_char(const void *pointer_to_uchar_a, const void *pointer_to_uchar_b) {
    unsigned char a = *(unsigned char *)(pointer_to_uchar_a);
    unsigned char b = *(unsigned char *)(pointer_to_uchar_b);
    return a < b ? -1 : a == b ? 0 : 1;
}

void test_normalize(void) {
    TEST_ASSERT_EQUAL_INT(-1, normalize(INT_MIN));
    TEST_ASSERT_EQUAL_INT(-1, normalize(-1));
    TEST_ASSERT_EQUAL_INT(0, normalize(0));
    TEST_ASSERT_EQUAL_INT(1, normalize(1));
    TEST_ASSERT_EQUAL_INT(1, normalize(INT_MAX));
}

void test_trim_string_works_with_empty_strings(void) {
    const char *input = "";
    const char *expected = input; /* Use a more meaningful name. */
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_trim_string_works_with_already_trimmed_strings(void) {
    const char *input = "a b";
    const char *expected = input; /* Use a more meaningful name. */
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_trim_string_properly_trims_preceding_spaces(void) {
    const char *input = "  a b";
    const char *expected = "a b";
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_trim_string_properly_trims_trailing_spaces(void) {
    const char *input = "a b  ";
    const char *expected = "a b";
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_trim_string_properly_trims_space_padded_strings(void) {
    const char *input = "  a b  ";
    const char *expected = "a b";
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_trim_string_works_with_strings_of_whitespaces(void) {
    const char *input = " \t \n ";
    const char *expected = "";
    char buffer[64];
    strcpy(buffer, input);
    trim_string(buffer);
    TEST_ASSERT_EQUAL_STRING(expected, buffer);
}

void test_read_integers(void) {
    char filename[] = "test_read_integers_file.txt";
    FILE *file = fopen(filename, "w+");

    const int expected_count = 1;
    const int expected_value = 65535;

    int actual_values[1];
    int actual_count;

    fprintf(file, "%d", expected_value);
    fflush(file);

    actual_count = read_integers(filename, actual_values, expected_count);

    TEST_ASSERT_EQUAL(expected_count, actual_count);
    TEST_ASSERT_EQUAL(expected_value, actual_values[0]);

    remove(filename);
}

void test_compare_unsigned_char(void) {
    const unsigned char a = 0;
    const unsigned char b = 1;
    const unsigned char c = 2;
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&a), (void *)(&b)));
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&a), (void *)(&c)));
    TEST_ASSERT_EQUAL_INT8(-1, compare_unsigned_char((void *)(&b), (void *)(&c)));

    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&a), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&b), (void *)(&b)));
    TEST_ASSERT_EQUAL_INT8(0, compare_unsigned_char((void *)(&c), (void *)(&c)));

    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&b), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&c), (void *)(&a)));
    TEST_ASSERT_EQUAL_INT8(1, compare_unsigned_char((void *)(&c), (void *)(&b)));
}

void test_insertion_sort_with_empty_range(void) {
    unsigned char source[] = {0};
    insertion_sort(source, 0, sizeof(unsigned char), compare_unsigned_char);
}

void test_insertion_sort_with_an_odd_number_of_single_bytes(void) {
    unsigned char source[] = {2, 3, 1};
    const unsigned char sorted[] = {1, 2, 3};
    const size_t array_size = sizeof(source) / sizeof(unsigned char);
    insertion_sort(source, array_size, sizeof(unsigned char), compare_unsigned_char);
    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(sorted, source, array_size, "insertion sort failed to sort the input");
}

void test_insertion_sort_with_an_even_number_of_single_bytes(void) {
    unsigned char source[] = {4, 2, 3, 1};
    const unsigned char sorted[] = {1, 2, 3, 4};
    const size_t array_size = sizeof(source) / sizeof(unsigned char);
    insertion_sort(source, array_size, sizeof(unsigned char), compare_unsigned_char);
    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(sorted, source, array_size, "insertion sort failed to sort the input");
}

void test_reverse_with_empty_range(void) {
    unsigned char source[] = {0};
    reverse(source, 0, sizeof(unsigned char));
}

void test_reverse_with_an_odd_number_of_single_bytes(void) {
    unsigned char source[] = {2, 3, 1};
    const unsigned char sorted[] = {1, 3, 2};
    const size_t array_size = sizeof(source) / sizeof(unsigned char);
    reverse(source, array_size, sizeof(unsigned char));
    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(sorted, source, array_size, "reverse failed to reverse the input");
}

void test_reverse_with_an_even_number_of_single_bytes(void) {
    unsigned char source[] = {2, 3, 1, 4};
    const unsigned char sorted[] = {4, 1, 3, 2};
    const size_t array_size = sizeof(source) / sizeof(unsigned char);
    reverse(source, array_size, sizeof(unsigned char));
    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(sorted, source, array_size, "reverse failed to reverse the input");
}

void test_find_next_power_of_two_works_for_all_integers(void) {
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(0UL), 1UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1UL), 2UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2UL), 4UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(3UL), 4UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4UL), 8UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(5UL), 8UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(7UL), 8UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8UL), 16UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(9UL), 16UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(15UL), 16UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16UL), 32UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17UL), 32UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(31UL), 32UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(32UL), 64UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(33UL), 64UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(63UL), 64UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(64UL), 128UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(65UL), 128UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(127UL), 128UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(128UL), 256UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(129UL), 256UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(255UL), 256UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(256UL), 512UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(257UL), 512UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(511UL), 512UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(512UL), 1024UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(513UL), 1024UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1023UL), 1024UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1024UL), 2048UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1025UL), 2048UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2047UL), 2048UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2048UL), 4096UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2049UL), 4096UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4095UL), 4096UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4096UL), 8192UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4097UL), 8192UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8191UL), 8192UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8192UL), 16384UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8193UL), 16384UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16383UL), 16384UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16384UL), 32768UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16385UL), 32768UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(32767UL), 32768UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(32768UL), 65536UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(32769UL), 65536UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(65535UL), 65536UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(65536UL), 131072UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(65537UL), 131072UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(131071UL), 131072UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(131072UL), 262144UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(131073UL), 262144UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(262143UL), 262144UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(262144UL), 524288UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(262145UL), 524288UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(524287UL), 524288UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(524288UL), 1048576UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(524289UL), 1048576UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1048575UL), 1048576UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1048576UL), 2097152UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1048577UL), 2097152UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2097151UL), 2097152UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2097152UL), 4194304UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2097153UL), 4194304UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4194303UL), 4194304UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4194304UL), 8388608UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4194305UL), 8388608UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8388607UL), 8388608UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8388608UL), 16777216UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8388609UL), 16777216UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16777215UL), 16777216UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16777216UL), 33554432UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(16777217UL), 33554432UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(33554431UL), 33554432UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(33554432UL), 67108864UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(33554433UL), 67108864UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(67108863UL), 67108864UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(67108864UL), 134217728UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(67108865UL), 134217728UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(134217727UL), 134217728UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(134217728UL), 268435456UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(134217729UL), 268435456UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(268435455UL), 268435456UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(268435456UL), 536870912UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(268435457UL), 536870912UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(536870911UL), 536870912UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(536870912UL), 1073741824UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(536870913UL), 1073741824UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1073741823UL), 1073741824UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1073741824UL), 2147483648UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1073741825UL), 2147483648UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2147483647UL), 2147483648UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2147483648UL), 4294967296UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2147483649UL), 4294967296UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4294967295UL), 4294967296UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4294967296UL), 8589934592UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4294967297UL), 8589934592UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8589934591UL), 8589934592UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8589934592UL), 17179869184UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8589934593UL), 17179869184UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17179869183UL), 17179869184UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17179869184UL), 34359738368UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17179869185UL), 34359738368UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(34359738367UL), 34359738368UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(34359738368UL), 68719476736UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(34359738369UL), 68719476736UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(68719476735UL), 68719476736UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(68719476736UL), 137438953472UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(68719476737UL), 137438953472UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(137438953471UL), 137438953472UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(137438953472UL), 274877906944UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(137438953473UL), 274877906944UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(274877906943UL), 274877906944UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(274877906944UL), 549755813888UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(274877906945UL), 549755813888UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(549755813887UL), 549755813888UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(549755813888UL), 1099511627776UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(549755813889UL), 1099511627776UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1099511627775UL), 1099511627776UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1099511627776UL), 2199023255552UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1099511627777UL), 2199023255552UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2199023255551UL), 2199023255552UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2199023255552UL), 4398046511104UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2199023255553UL), 4398046511104UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4398046511103UL), 4398046511104UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4398046511104UL), 8796093022208UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4398046511105UL), 8796093022208UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8796093022207UL), 8796093022208UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8796093022208UL), 17592186044416UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(8796093022209UL), 17592186044416UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17592186044415UL), 17592186044416UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17592186044416UL), 35184372088832UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(17592186044417UL), 35184372088832UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(35184372088831UL), 35184372088832UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(35184372088832UL), 70368744177664UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(35184372088833UL), 70368744177664UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(70368744177663UL), 70368744177664UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(70368744177664UL), 140737488355328UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(70368744177665UL), 140737488355328UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(140737488355327UL), 140737488355328UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(140737488355328UL), 281474976710656UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(140737488355329UL), 281474976710656UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(281474976710655UL), 281474976710656UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(281474976710656UL), 562949953421312UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(281474976710657UL), 562949953421312UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(562949953421311UL), 562949953421312UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(562949953421312UL), 1125899906842624UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(562949953421313UL), 1125899906842624UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1125899906842623UL), 1125899906842624UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1125899906842624UL), 2251799813685248UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1125899906842625UL), 2251799813685248UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2251799813685247UL), 2251799813685248UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2251799813685248UL), 4503599627370496UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2251799813685249UL), 4503599627370496UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4503599627370495UL), 4503599627370496UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4503599627370496UL), 9007199254740992UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4503599627370497UL), 9007199254740992UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(9007199254740991UL), 9007199254740992UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(9007199254740992UL), 18014398509481984UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(9007199254740993UL), 18014398509481984UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(18014398509481983UL), 18014398509481984UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(18014398509481984UL), 36028797018963968UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(18014398509481985UL), 36028797018963968UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(36028797018963967UL), 36028797018963968UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(36028797018963968UL), 72057594037927936UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(36028797018963969UL), 72057594037927936UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(72057594037927935UL), 72057594037927936UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(72057594037927936UL), 144115188075855872UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(72057594037927937UL), 144115188075855872UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(144115188075855871UL), 144115188075855872UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(144115188075855872UL), 288230376151711744UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(144115188075855873UL), 288230376151711744UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(288230376151711743UL), 288230376151711744UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(288230376151711744UL), 576460752303423488UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(288230376151711745UL), 576460752303423488UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(576460752303423487UL), 576460752303423488UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(576460752303423488UL), 1152921504606846976UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(576460752303423489UL), 1152921504606846976UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1152921504606846975UL), 1152921504606846976UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1152921504606846976UL), 2305843009213693952UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(1152921504606846977UL), 2305843009213693952UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2305843009213693951UL), 2305843009213693952UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2305843009213693952UL), 4611686018427387904UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(2305843009213693953UL), 4611686018427387904UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4611686018427387903UL), 4611686018427387904UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4611686018427387904UL), 9223372036854775808UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(4611686018427387905UL), 9223372036854775808UL);
    TEST_ASSERT_EQUAL_INT64(find_next_power_of_two(9223372036854775807UL), 9223372036854775808UL);
}

void test_random_integer_respects_the_provided_range(void) {
    const int min_min = -3;
    const int max_min = 0;
    const int min_max = 0;
    const int max_max = 4;
    int min;
    int max;
    int random_result;
    for (min = min_min; min < max_min; min++) {
        for (max = min_max; max < max_max; max++) {
            random_result = random_integer(min, max);
            TEST_ASSERT_TRUE(min <= random_result && random_result <= max);
        }
    }
}

int main(void) {
    UNITY_BEGIN();
    log_message("Started running tests");
    RUN_TEST(test_normalize);
    RUN_TEST(test_trim_string_works_with_empty_strings);
    RUN_TEST(test_trim_string_works_with_already_trimmed_strings);
    RUN_TEST(test_trim_string_properly_trims_preceding_spaces);
    RUN_TEST(test_trim_string_properly_trims_trailing_spaces);
    RUN_TEST(test_trim_string_properly_trims_space_padded_strings);
    RUN_TEST(test_trim_string_works_with_strings_of_whitespaces);
    RUN_TEST(test_read_integers);
    RUN_TEST(test_compare_unsigned_char);
    RUN_TEST(test_insertion_sort_with_empty_range);
    RUN_TEST(test_insertion_sort_with_an_odd_number_of_single_bytes);
    RUN_TEST(test_insertion_sort_with_an_even_number_of_single_bytes);
    RUN_TEST(test_reverse_with_empty_range);
    RUN_TEST(test_reverse_with_an_odd_number_of_single_bytes);
    RUN_TEST(test_reverse_with_an_even_number_of_single_bytes);
    RUN_TEST(test_find_next_power_of_two_works_for_all_integers);
    RUN_TEST(test_random_integer_respects_the_provided_range);
    log_message("Finished running tests");
    return UNITY_END();
}
