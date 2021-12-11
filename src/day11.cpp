#include <stdio.h>

#include "file.h"
#include "stack.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint8_t OCTO_MAX = 100;
static const uint8_t SIDE_SIZE = 10;
static const uint8_t FLASH_THRESHOLD = 9;
static const uint8_t TOP_IDX = SIDE_SIZE-1;
static const uint8_t BOTTOM_IDX = (SIDE_SIZE*(SIDE_SIZE-1));

typedef struct Parser {
    void init();
    void destroy();
    bool add_line(const char* str);
    uint16_t step_n(const uint16_t n_steps);
    uint16_t flashes() const { return _n_flashes; }
    uint16_t step_until_sync();


private:
    bool add_visit(const uint8_t n);
    Stack<uint8_t> _stack;
    uint8_t _octopuses[OCTO_MAX];
    uint8_t _n_row;
    uint16_t _n_flashes;

} Parser;

bool Parser::add_line(const char* str) {
    uint8_t it = 0;
    if (_n_row >= SIDE_SIZE) return false;
    while (true) {
        if (str[it] == '\0') {
            break;
        } else if (ascii_isdigit(str[it])) {
            if (it >= SIDE_SIZE) return false;
            _octopuses[_n_row*SIDE_SIZE + it] = str[it] - '0';
        } else {
            return false;
        }
        ++it;
    }
    _n_row++;
    return true;
}

void Parser::init() {
    _n_row = 0;
    _n_flashes = 0;
    _stack.init(128);
}

void Parser::destroy() {
    _stack.destroy();
}

bool Parser::add_visit(const uint8_t n) {
    // already flashed this turn
    if (_octopuses[n] == 0) return true;
    _octopuses[n] += 1;
    if (_octopuses[n] > FLASH_THRESHOLD) return _stack.push(n);
    return true;
}

uint16_t Parser::step_until_sync() {
    // we already did 100
    uint16_t steps = 100;
    steps += step_n(65535); // loop for as long as we can

    return steps;
}

uint16_t Parser::step_n(const uint16_t n_steps) {
    if (_n_row != SIDE_SIZE) return false;

    uint16_t step;
    for (step = 0; step < n_steps; ++step) {
        int step_flashes = 0;
        // increase all energy by one
        for (uint8_t i = 0; i < OCTO_MAX; ++i) {
            _octopuses[i] += 1;
            if(_octopuses[i] > FLASH_THRESHOLD) {
                if(!_stack.push(i)) return 0;
            }
        }
        // handle flashes
        uint8_t n;
        while (_stack.pop(n)) {
            // not flashed or already flashed
            if (_octopuses[n] == 0) continue;

            _octopuses[n] = 0;
            step_flashes += 1;

            // flash the 8 adjacent
            const bool has_left = (n % SIDE_SIZE) != 0;
            const bool has_right = (n % (SIDE_SIZE)) != 9;
            const bool has_top = (n > TOP_IDX);
            const bool has_down = (n < BOTTOM_IDX);
            // handle top
            if (has_top) {
                add_visit(n-SIDE_SIZE);
                if (has_left) add_visit(n-SIDE_SIZE-1);
                if (has_right) add_visit(n-SIDE_SIZE+1);
            }
            // handle bottom
            if (has_down) {
                add_visit(n+SIDE_SIZE);
                if (has_left) add_visit(n+SIDE_SIZE-1);
                if (has_right) add_visit(n+SIDE_SIZE+1);
            }
            // handle sides
            if (has_left) add_visit(n-1);
            if (has_right) add_visit(n+1);
        }

        _n_flashes += step_flashes;
        // return for Part 2 if all octopuses flashed
        if (step_flashes == OCTO_MAX) return step+1;
    }
    return step;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Parser parser;
    parser.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!parser.add_line(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    if (parser.step_n(100) != 100) return -1;
    const uint16_t answer1 = parser.flashes();
    const uint16_t answer2 = parser.step_until_sync();

    file.close();
    parser.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 11 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
