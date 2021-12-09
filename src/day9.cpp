#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint8_t MAX_ROW = 100;
static const uint8_t N_COL = 100;


typedef struct Heightmap {
    void init();
    uint32_t low_points_risk() const;
    bool add_row(const char* str);

private:
    uint8_t _map[MAX_ROW][N_COL];
    uint8_t _n_row;

} Heightmap;

void Heightmap::init() {
    _n_row = 0;
}

uint32_t Heightmap::low_points_risk() const {
    uint32_t risk = 0;
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
    const uint32_t answer2 = 0;

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 9 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
