#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t LINE_LENGTH = 12;

static bool extract_digit(const char* str, int* out) {
    size_t i = 0;
    while (str[i] != ' ') {
        if (str[i] == '\0') return false;
        ++i;
    }
    if (!ascii_isdigit(str[++i])) return false;

    *out = str[i] - '0';
    return true;
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
    int horizontal_pos = 0;
    int depth = 0;
    int depth2 = 0;
    int aim = 0;
    while (file.readline(str, LINE_LENGTH)) {
        int val;
        if (!extract_digit(str, &val)) continue;

        switch(str[0]) {
            case 'f': 
                horizontal_pos += val;
                depth2 += (aim * val);
                break;
            case 'd':
                depth += val;
                aim += val;
                break;
            case 'u':
                depth -= val;
                aim -= val;
                break;
            default: continue;
        }
    }

    const int answer1 = horizontal_pos * depth;
    const int answer2 = horizontal_pos * depth2;

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 2 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
