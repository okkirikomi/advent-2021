#include <bitset>
#include <stdio.h>
#include <string.h>

#include "bitset.h"
#include "file.h"
#include "strtoint.h"
#include "timer.h"

const uint16_t INPUT_MAX = 256;
const uint8_t MAX_Y = 137;
const uint8_t MAX_X = 139;
const uint16_t MAX_CUCUMBERS = 4800;

typedef struct Pos {
    uint8_t x, y;
} Pos;

typedef struct Cucumbers {
    void init();
    void destroy();
    bool read_map(const char* str);
    uint16_t move();
    void print() const;

private:
    std::bitset<MAX_X> _occupied[MAX_Y];
    uint8_t _row;

    Pos _east[MAX_CUCUMBERS];
    uint16_t _n_east;
    Pos _south[MAX_CUCUMBERS];
    uint16_t _n_south;
} Cucumbers;

void Cucumbers::init() {
    _row = 0;
    _n_east = 0;
    _n_south = 0;
}

void Cucumbers::destroy() {
    // nothing
}

void Cucumbers::print() const {
    const size_t BUF_SIZE = sizeof(char)*MAX_Y*MAX_X + MAX_Y + 1;
    char buffer[BUF_SIZE];
    memset(buffer, '.', sizeof(buffer));
    for (uint8_t i = 0; i < _n_east; ++i) {
        buffer[_east[i].x+_east[i].y*(MAX_X+1)] = '>';
    }
    for (uint8_t i = 0; i < _n_south; ++i) {
        buffer[_south[i].x+_south[i].y*(MAX_X+1)] = 'v';
    }
    for (uint8_t i = 0; i < MAX_Y; ++i) {
        buffer[(MAX_X)+i*(MAX_X+1)] = '\n';
    }
    buffer[BUF_SIZE-1] = '\0';
    printf("%s\n", buffer);
}

// BETTER, this can be much faster if we pack
// everything in a bit
uint16_t Cucumbers::move() {
    uint16_t step = 0;
    bool moved = true;
    std::bitset<MAX_X> buffer[MAX_Y];

    memcpy(buffer, _occupied, sizeof(_occupied));

    while (moved) {
        moved = false;
        // go through east first
        for (uint16_t i = 0; i < _n_east; ++i) {
            // check if it can move
            const uint8_t x = (_east[i].x == (MAX_X - 1))? 0 : _east[i].x + 1;
            if (_occupied[_east[i].y][x] == true) continue;
            // set new pos on and old pos off
            buffer[_east[i].y].set(x, 1);
            buffer[_east[i].y].set(_east[i].x, 0);
            // update our cucumber
            _east[i].x = x;
            moved = true;
        }
        if (moved) memcpy(_occupied, buffer, sizeof(_occupied));
        // then south
        for (uint16_t i = 0; i < _n_south; ++i) {
            // check if it can move
            const uint8_t y = (_south[i].y == (MAX_Y - 1))? 0 : _south[i].y + 1;
            if (_occupied[y][_south[i].x] == true) continue;
            // set new pos on and old pos off
            buffer[y].set(_south[i].x, 1);
            buffer[_south[i].y].set(_south[i].x, 0);
            // update our cucumber
            _south[i].y = y;
            moved = true;
        }
        if (moved) memcpy(_occupied, buffer, sizeof(_occupied));
        ++step;
    }
    return step;
}

bool Cucumbers::read_map(const char* str) {
    if (_row == MAX_Y) return false;
    uint8_t it = 0;
    while (str[it] != 0) {
        if (str[it] == '>') {
            if (_n_east == MAX_CUCUMBERS) return false;
            _east[_n_east].x = it;
            _east[_n_east].y = _row;
            _occupied[_row].set(it, 1);
            _n_east += 1;
        } else if (str[it] == 'v') {
            if (_n_south == MAX_CUCUMBERS) return false;
            _south[_n_south].x = it;
            _south[_n_south].y = _row;
            _occupied[_row].set(it, 1);
            _n_south += 1;
        } else if (str[it] != '.') return false;
        it += 1;
    }
    if (it != MAX_X) return false;
    _row += 1;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Cucumbers cucumbers;
    cucumbers.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX) > 1) {
        if (!cucumbers.read_map(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    const uint16_t answer1 = cucumbers.move();

    file.close();
    cucumbers.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 25 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);

    return 0;
}
