#include <stdio.h>

#include "file.h"
#include "ring_buffer.h"
#include "strtoint.h"
#include "timer.h"

static const size_t LINE_LENGTH = 8;

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

    Ringbuffer<int> ring_buffer;
    ring_buffer.init(3);

    char str[LINE_LENGTH];
    int larger = 0;
    int larger_sums = 0;
    int last_sum = 0;
    int line_count = 0;

    while (file.readline(str, LINE_LENGTH)) {
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

    file.close();
    ring_buffer.free();

    const uint64_t completion_time = timer_stop();
    printf("Day 1 completion time: %" PRIu64 "ms\n", completion_time);
    printf("Larger measurement: %i\n", larger);
    printf("Larger 3 sums measurement: %i\n", larger_sums);

    return 0;
}
