#include <bitset>
#include <limits.h>
#include <stdio.h>

#include "file.h"
#include "timer.h"

static const size_t LINE_LENGTH = 13;
static const size_t INPUT_LENGTH = LINE_LENGTH-1;
static const int REPORT_MAX_LENGTH = 1000;

typedef std::bitset<REPORT_MAX_LENGTH> diagnostic_bitset;

static int most_at_bit(const diagnostic_bitset& line, const size_t n_lines) {
    const size_t n_set = line.count();
    if (n_set * 2 >= n_lines) return 1;
    return 0;
}

static uint power_consumption(const diagnostic_bitset* input) {
    uint gamma = 0;
    uint epsilon = 0;
    for (size_t i = 0; i < INPUT_LENGTH; ++i) {
        const int most = most_at_bit(input[i], REPORT_MAX_LENGTH);
        if (most) gamma += 1;
        else epsilon += 1;
        if (i + 1 < INPUT_LENGTH) {
            gamma <<= 1;
            epsilon <<= 1;
        }
    }
    return gamma * epsilon;
}

static uint calc_rating(const diagnostic_bitset* input, const int pivot_value = 1) {
    diagnostic_bitset mask_remaining;
    diagnostic_bitset bits_line;
    diagnostic_bitset xor_mask;

    // we start with all lines
    mask_remaining.set();
    for (size_t i = 0; i < INPUT_LENGTH; ++i) {
        // calc the most common bit for position i
        bits_line = input[i] & mask_remaining;
        const int most = most_at_bit(bits_line, mask_remaining.count());

        // filter out lines not matching the most common bit at position i
        if (most == pivot_value) xor_mask.set();
        else xor_mask.reset();
        xor_mask ^= input[i];
        mask_remaining &= xor_mask;

        if (mask_remaining.count() <= 1) break;
    }

    // find the column index where our answer reside
    size_t column_id = 0;
    while (column_id < REPORT_MAX_LENGTH) {
        if (mask_remaining.test(column_id)) break;
        ++column_id;
    }

    // binary to decimal
    uint result = 0;
    for (size_t i = 0; i < INPUT_LENGTH; ++i) {
        if (input[i][column_id]) result += 1;
        if (i + 1 < INPUT_LENGTH) result <<= 1;
    }
    return result;
}

static uint life_support_rating(const diagnostic_bitset* input) {
    const uint oxygen_rating = calc_rating(input);
    const uint scrubber_rating = calc_rating(input, 0);
    return oxygen_rating * scrubber_rating;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    diagnostic_bitset input[INPUT_LENGTH];
    char str[LINE_LENGTH];
    int n_line = 0;
    while (file.readline(str, LINE_LENGTH)) {
        for (size_t i = 0; i < INPUT_LENGTH; ++i) {
            if (str[i] == '1') input[i].set(n_line);
        }
        ++n_line;
    }

    const uint answer1 = power_consumption(input);
    const uint answer2 = life_support_rating(input);

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 3 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %u\n", answer2);

    return 0;
}
