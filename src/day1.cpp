#include <stdio.h>

#include "file.h"
#include "ring_buffer.h"
#include "strtoint.h"
#include "timer.h"

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    FILE* input = open_file(argv[1]);
    if (!input) {
        printf("No input!\n");
        return -1;
    }

    Ringbuffer<int> ring_buffer;
    ring_buffer.init(3);

    char str[32];
    int larger = 0;
    int larger_sums = 0;
    int last_sum = 0;
    int line_count = 0;
    while (fgets(str, 32, input)) {
        ++line_count;
        const int depth = strtoint(str);
        if (line_count > 1 && ring_buffer.last() < depth) ++larger;

        ring_buffer.push(depth);
        if (line_count > 2) {
            const int sum = ring_buffer.sum();
            if (last_sum && last_sum < sum) ++larger_sums;
            last_sum = sum;
        }
    }

    close_file(input);
    ring_buffer.free();

    printf("Day 1 completion time: %" PRIu64 "ms\n", timer_stop());
    printf("Larger measurement: %i\n", larger);
    printf("Larger 5 sums measurement: %i\n", larger_sums);

    return 0;
}
