#include <stdio.h>

#include "file.h"
#include "stack.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint8_t MAX_ROW = 100;
static const uint8_t N_COL = 100;
static const uint8_t MAX_POINTS = 0xFF;

typedef struct Pos {
    uint8_t x, y;
} Pos;

typedef struct Heightmap {
    void init();
    void destroy();
    uint16_t low_points_risk();
    uint32_t largest_basins();
    bool add_row(const char* str);

private:
    Pos _low_points[MAX_POINTS];
    Stack<Pos> _stack;
    uint8_t _map[MAX_ROW][N_COL];
    uint8_t _n_row;
    uint8_t _n_lows;

} Heightmap;

void Heightmap::destroy() {
    _stack.destroy();
}

void Heightmap::init() {
    _n_row = 0;
    _n_lows = 0;
    _stack.init(32);
}

uint32_t Heightmap::largest_basins() {
    uint16_t a = 0;
    uint16_t b = 0;
    uint16_t c = 0;
    for (uint8_t i = 0; i < _n_lows; ++i) {
        uint16_t basin_size = 1;
        _stack.clear();
        // we set to 9 to mark visited
        _map[_low_points[i].y][_low_points[i].x] = 9;
        _stack.push(std::move(_low_points[i]));

        // visit neighbours as long as they are not 9
        Pos visiting;
        while (_stack.pop(visiting)) {        
            if (visiting.x != 0 && 9 != _map[visiting.y][visiting.x-1]) {
                Pos to_visit;
                to_visit.x = visiting.x-1;
                to_visit.y = visiting.y;
                _map[to_visit.y][to_visit.x] = 9;
                _stack.push(std::move(to_visit));
                basin_size += 1;
            }
            if (visiting.y != 0 && 9 != _map[visiting.y-1][visiting.x]) {
                Pos to_visit;
                to_visit.x = visiting.x;
                to_visit.y = visiting.y-1;
                _map[to_visit.y][to_visit.x] = 9;
                _stack.push(std::move(to_visit));
                basin_size += 1;
            }
            if (visiting.x < N_COL - 1 && 9 != _map[visiting.y][visiting.x+1]) {
                Pos to_visit;
                to_visit.x = visiting.x+1;
                to_visit.y = visiting.y;
                _map[to_visit.y][to_visit.x] = 9;
                _stack.push(std::move(to_visit));
                basin_size += 1;
            }
            if (visiting.y < _n_row - 1 && 9 != _map[visiting.y+1][visiting.x]) {
                Pos to_visit;
                to_visit.x = visiting.x;
                to_visit.y = visiting.y+1;
                _map[to_visit.y][to_visit.x] = 9;
                _stack.push(std::move(to_visit));
                basin_size += 1;
            }
        }
        // BETTER, got to be a better way
        if(basin_size > a) {
            c = b;
            b = a;
            a = basin_size;
        } else if(basin_size > b) {
            c = b;
            b = basin_size;
        } else if(basin_size > c) {
            c = basin_size;
        }
    }
    return a * b * c;
}

uint16_t Heightmap::low_points_risk() {
    uint16_t risk = 0;
    for (uint8_t y = 0; y < _n_row; ++y) {
        for (uint8_t x = 0; x < N_COL; ++x) {
            // check the 4 neighbors
            const uint8_t& height = _map[y][x];
            if (x != 0 && height >= _map[y][x-1])
                continue;

            if (y != 0 && height >= _map[y-1][x])
                continue;

            if (x < N_COL - 1 && height >= _map[y][x+1])
                continue;

            if (y < _n_row - 1 && height >= _map[y+1][x])
                continue;

            risk += 1 + height;

            if (_n_lows == MAX_POINTS) continue;
            _low_points[_n_lows].x = x;
            _low_points[_n_lows].y = y;
            ++_n_lows;
        }
    }
    return risk;
}

// simple row of digits:
// 2199943210
bool Heightmap::add_row(const char* str) {
    uint8_t it = 0;
    if (_n_row >= MAX_ROW) return false;
    while (str[it] != '\0') {
        if (ascii_isdigit(str[it])) {
            if (it >= N_COL) return false;
            _map[_n_row][it] = str[it] - '0';
        }
        ++it;
    }
    _n_row++;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Heightmap height_map;
    height_map.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!height_map.add_row(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    const uint16_t answer1 = height_map.low_points_risk();
    const uint32_t answer2 = height_map.largest_basins();

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 9 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
