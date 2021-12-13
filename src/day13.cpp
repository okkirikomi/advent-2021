#include <stdio.h>
#include <string.h>
#include <unordered_set>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 32;
static const uint16_t MAX_X = 1311;
static const uint16_t MAX_Y = 895;
static const uint32_t MAX_DOTS = 866;

typedef struct Dot {
    uint16_t x;
    uint16_t y;
    bool operator== (const Dot& d) const {
        return x == d.x && y == d.y;
    }
} Dot;

struct hash_func {
    size_t operator() (const Dot& d) const {
        return d.x + d.y * MAX_X;
    }
};

typedef struct Paper {
    void init();

    bool add_dot(const char* str);
    bool fold(const char* str);
    void print_all() const;
    uint32_t visible_count();

private:
    Dot _dots[MAX_DOTS];
    uint16_t _actual_x;
    uint16_t _actual_y;
    uint16_t _n_points;
    // we use a set to remove duplicate dots
    // BETTER, don't use std
    std::unordered_set<Dot, hash_func> _set;

} Paper;

void Paper::init() {
    _actual_x = MAX_X;
    _actual_y = MAX_Y;
    _n_points = 0;
    _set.reserve(MAX_DOTS);
}

uint32_t Paper::visible_count() {
    for (uint16_t i = 0; i < _n_points; ++i) {
        _set.insert(_dots[i]);
    }
    return _set.size();
}

void Paper::print_all() const {
    // build the buffer to display
    size_t size = sizeof(char)*_actual_y*_actual_x + _actual_y + 1;
    char* buffer = (char*) malloc(size);
    memset(buffer, '.', size);
    // set the new lines
    for (uint16_t y = 0; y < _actual_y; ++y) {
        buffer[(_actual_x)+y*(_actual_x+1)] = '\n';
    }
    // set the dots
    for (uint16_t i = 0; i < _n_points; ++i) {
        if (_dots[i].x >= _actual_x || _dots[i].y >= _actual_y) continue;
        const uint16_t it = _dots[i].x+_dots[i].y*(_actual_x+1);
        buffer[it] = '#';
    }
    // we're done
    buffer[size-1] = '\0';
    printf("%s\n", buffer);
    free(buffer);
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

    // BETTER, check if removing the duplicate dots is faster
    if (axis == 'y') {
        for (uint16_t i = 0; i < _n_points; ++i) {
            if (_dots[i].y <= value) continue;
            _dots[i].y = value - (_dots[i].y - value);
        }
        _actual_y = value;
    } else {
        for (uint16_t i = 0; i < _n_points; ++i) {
            if (_dots[i].x <= value) continue;
            _dots[i].x = value - (_dots[i].x - value);
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

    if (_n_points == MAX_DOTS) return false;
    _dots[_n_points].x = value[0];
    _dots[_n_points].y = value[1];
    ++_n_points;
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

    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 =\n");
    paper.print_all();

    // For this problem, printing is part of the answer and might not be trivial,
    // so we stop timer after.
    const uint64_t completion_time = timer_stop();
    printf("Day 13 completion time: %" PRIu64 "µs\n", completion_time);

    return 0;
}
