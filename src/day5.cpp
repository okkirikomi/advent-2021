#include <assert.h>
#include <cmath>
#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t INPUT_MAX = 32;
static const size_t GRID_SIDE = 1000;

enum LINE_TYPE { DIAGONAL, NOT_DIAGONAL };

// We keep an array of all positions on grid containing the
// number of vents at that position.
// To avoid running the problem twice to provide answers
// with and without counting diagonals (part 1 and 2),
// we count them both in low and high part of 8 bits integer
typedef struct Grid {
    void init(const size_t size);
    void destroy();
    bool add_vents(const char* str);
    uint overlap_count(const LINE_TYPE type) const;
    void set(const uint x, const uint y, const LINE_TYPE type);

private:
    uint8_t* _data;
    size_t _size;
    uint _n_overlap;
    uint _n_overlap_diagonal;
} Grid;

// To avoid parsing the whole grid to count the overlapping vents,
// we count them as we mark them.
// Computing both parts of the problem at the same time makes this ugly.
// First 3 low bits are used for part one (straight lines) count flag.
// First 3 high bits are used for part two (everything) count flag.
//
// Flag values:
// 0 = empty cell, 1 = one vent, 2 = already counted
//
// The part two count also considers the straight lines.
// Whenever we have an overlap on straight lines, we increment both counts.
// If get one straight mark, we increment count #2 if there was one already.
// For diagonal marks, same logic applies.
void Grid::set(const uint x, const uint y, const LINE_TYPE type) {
    uint8_t& cell = _data[x+y*GRID_SIDE];
    const uint8_t low_check = (cell & 0x03);
    const uint8_t high_check = (cell & 0x30);

    if (type == NOT_DIAGONAL) {
        if (low_check == 0x1) {
            _n_overlap += 1;
            cell += 0x1;
            if (high_check < 0x20) {
                _n_overlap_diagonal += 1;
                cell += (0x20 - high_check);
            }
        } else {
            cell += 0x1;
            if (high_check == 0x10) {
                _n_overlap_diagonal += 1;
                cell += (0x20 - high_check);
            }
        }
    } else {
        if ((low_check == 0x1 && high_check == 0x00) || (low_check == 0x0 && high_check == 0x10)) {
            cell += (0x20 - high_check);
            _n_overlap_diagonal += 1;
        } else {
            cell += 0x10;
        }
    }
}

void Grid::init(const size_t size) {
    assert(size > 0);
    _size = size;
    _data = (uint8_t*) calloc(sizeof(uint8_t), _size);
    if (_data == NULL) abort();

    _n_overlap = 0;
    _n_overlap_diagonal = 0;
}

void Grid::destroy() {
    free(_data);
}

uint Grid::overlap_count(const LINE_TYPE type = NOT_DIAGONAL) const {
    if (type == DIAGONAL) return _n_overlap_diagonal;
    else return _n_overlap;
}

// We expect a line in the form of:
// x1,y1 -> x2,y2
// 800,363 -> 800,25
bool Grid::add_vents(const char* str) {
    size_t it = 0;
    uint value = 0;

    // x1, y2, x1, y2;
    int points[4];
    size_t points_i = 0;
    // parse and extract the values
    while (str[it] != '\0') {
        if ((it > 0 && str[it-1] == ',') ||
            (it > 0 && str[it] == ' ' && str[it+1] == '-')) {
            points[points_i++] = value;
            if (points_i == 4) return false;
            value = 0;
        }
        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            value *= 10;
            if (value >= GRID_SIDE) return false;
            value += c;
        }
        ++it;
    }
    points[points_i++] = value;

    // didn't get 4 values, bail out
    if (points_i != 4) return false;

    // we have a diagonal if one of the axis is not constant
    if (points[0] != points[2] && points[1] != points[3]) {
        // handle diagonal only at 45 degrees
        if (abs(points[0] - points[2]) != abs(points[1] - points[3])) return false;

        // not pretty but simple enough to handle all 4 directions
        // we don't need to check the boundary of the y array if the two previous ifs conditions are cleared
        if (points[0] < points[2])  {
            if (points[1] < points[3]) {
                for (int x = points[0], y = points[1]; x <= points[2]; ++x, ++y) set(x, y, DIAGONAL);
            } else {
                for (int x = points[0], y = points[1]; x <= points[2]; ++x, --y) set(x, y, DIAGONAL);
            }
        } else {
            if (points[1] < points[3]) {
                for (int x = points[0], y = points[1]; x >= points[2]; --x, ++y) set(x, y, DIAGONAL);
            } else {
                for (int x = points[0], y = points[1]; x >= points[2]; --x, --y) set(x, y, DIAGONAL);
            }
        }
        return true;
    }

    // we have a straight line then
    // set it up so we always increment x1 -> x2
    int x1, x2, y1, y2;
    if (points[0] < points[2]) {
        x1 = points[0];
        x2 = points[2];
    } else {
        x1 = points[2];
        x2 = points[0];
    }
    if (points[1] < points[3]) {
        y1 = points[1];
        y2 = points[3];
    } else {
        y1 = points[3];
        y2 = points[1];
    }

    // fill x1 -> x2 or y1 -> y2
    if (x1 != x2) for(int x = x1; x <= x2; ++x) set(x, points[1], NOT_DIAGONAL);
    else for(int y = y1; y <= y2; ++y) set(points[0], y, NOT_DIAGONAL);

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

    Grid grid;
    grid.init(GRID_SIDE*GRID_SIDE);

    char str[INPUT_MAX];
    int n_line = 0;
    while (true) {
        const int n_read = file.readline(str, INPUT_MAX);
        if (n_read == 0) break;
        grid.add_vents(str);
        ++n_line;
    }

    const uint answer1 = grid.overlap_count(NOT_DIAGONAL);
    const uint answer2 = grid.overlap_count(DIAGONAL);

    grid.destroy();
    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 5 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
