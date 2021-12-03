#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "bitset.h"
#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t LINE_LENGTH = 6;
static const size_t INPUT_LENGTH = LINE_LENGTH-1;
static uint MASK = 0;

static uint calc_epsilon(const uint gamma) {
    uint epsilon = gamma;
    return BITMASK_FLIP(epsilon, MASK);
}

static uint calc_gamma(const int* first_bit_sum, const int n_line) {
    uint gamma = 0;
    const int to_beat = n_line / 2;
    for (size_t i = 0; i < INPUT_LENGTH; ++i) {
        if (first_bit_sum[i] > to_beat) {
            gamma += 1;
        }
        MASK+=1;
        if (i+1 < INPUT_LENGTH) {
            gamma <<= 1;
            MASK <<= 1;
        }
    }
    return gamma;
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

    char str[LINE_LENGTH];
    int n_line = 0;
    int first_bit_sum[INPUT_LENGTH] = {};
    while (file.readline(str, LINE_LENGTH)) {
        ++n_line;
        for (size_t i = 0; i < INPUT_LENGTH; ++i) {
            if (str[i] == '1') first_bit_sum[i] += 1;
        }
    }

    const uint gamma = calc_gamma(first_bit_sum, n_line);
    const uint epsilon = calc_epsilon(gamma);
    const uint answer1 = gamma * epsilon;

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 1 completion time: %" PRIu64 "ms\n", completion_time);
    printf("Answer 1 = %u\n", answer1);
    //printf("Answer 2 = %i\n", answer2);

    return 0;
}
