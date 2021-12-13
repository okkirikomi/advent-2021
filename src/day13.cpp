#include <stdio.h>
#include <string.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 32;
static const uint16_t MAX_X = 1311;
static const uint16_t MAX_Y = 895;
static const uint32_t MAX_DOTS = MAX_X * MAX_Y;

typedef struct Paper {
    void init();

    bool add_dot(const char* str);
    bool fold(const char* str);

    void print_all();


    uint32_t visible_count() const;

private:
    uint8_t _dots[MAX_Y][MAX_X];
    uint16_t _actual_x;
    uint16_t _actual_y;

} Paper;

void Paper::init() {
    memset(_dots, 0, sizeof(_dots));
    _actual_x = MAX_X;
    _actual_y = MAX_Y;
}

// FIXME, keep the count as we are folding?
uint32_t Paper::visible_count() const {
    uint32_t count = 0;
    for (uint16_t y = 0; y < _actual_y; ++y)
    for (uint16_t x = 0; x < _actual_x; ++x)
    if (_dots[y][x] != 0) ++count;

    return count;
}

void Paper::print_all() {
    for (uint16_t y = 0; y < _actual_y; ++y) {
        for (uint16_t x = 0; x < _actual_x; ++x) {
            if (_dots[y][x] == 0) printf(".");
            else printf("#");
        }
        printf("\n");
    }
    printf("\n");
}

// expecting:
// fold along y=7
// fold along x=5
bool Paper::fold(const char* str) {
    // let's assume we receive valid data beyond this
    if (str[0] != 'f') return false;

    size_t it = 0;
    char axis = 0;
    while (str[it] != '\0') {
        if (str[it] =='x' || str[it] == 'y') break;
        ++it;
    }

    axis = str[it];
    it += 2; // unsafe

    uint16_t value = 0;
    while (str[it] != '\0') {
        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            value *= 10; // this can overflow
            value += c;
        }
        ++it;
    }

    // FIXME, make this simpler?
    if (axis == 'y') {
        for (uint16_t y = _actual_y - 1; y > value; --y) {
            for (uint16_t x = 0; x < _actual_x; ++x) {
                _dots[_actual_y-y-1][x] += _dots[y][x];
            }
        }
        _actual_y = value;
    } else {
        for (uint16_t x = _actual_x - 1; x > value; --x) {
            for (uint16_t y = 0; y < _actual_y; ++y) {
                _dots[y][_actual_x-x-1] += _dots[y][x];
            }
        }
        _actual_x = value;
    }
    return true;
}

bool Paper::add_dot(const char* str) {
    size_t it = 0;
    uint16_t value[2] = { 0 };

    for (uint16_t i = 0; i < 2; ++i) {
        while (str[it] != '\0') {
            if (str[it] == ',') {
                ++it;
                break;
            }
            if (ascii_isdigit(str[it])) {
                const char c = str[it] - '0';
                value[i] *= 10; // this can overflow
                value[i] += c;
            }
            ++it;
        }
    }

    _dots[value[1]][value[0]] = 1;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Paper paper;
    paper.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX) > 1) {
        if (!paper.add_dot(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    // one instruction
    if(file.readline(str, INPUT_MAX)) paper.fold(str);
    const uint32_t answer1 = paper.visible_count();

    // fold the rest
    while (file.readline(str, INPUT_MAX)) {
        paper.fold(str);
    }

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 13 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 =\n");
    paper.print_all();

    return 0;
}
