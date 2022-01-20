#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bitset.h"
#include "file.h"
#include "strtoint.h"
#include "timer.h"

// FIXME
//#ifndef UINT128MAX
//#error "no __uint128_t"
//#endif

const uint16_t INPUT_MAX = 256;
const uint8_t MAX_X = 139;
const uint8_t MAX_Y = 136;
const uint16_t MAX_CUCUMBERS = 4800;


#define N 256

    uint64_t _p[N][N/32];
    uint64_t _p2[N][N/32];
    uint64_t (*p)[N/32];
    uint64_t (*p2)[N/32];

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
    void set(const int x, const int y, const int v);
    int get(const int x, const int y);
    uint8_t _row;


} Cucumbers;

void Cucumbers::init() {
    _row = 0;
    p = _p + 1;
    p2 = _p2 + 1;
}

void Cucumbers::destroy() {
    // nothing
}

int Cucumbers::get(const int x, const int y) {
    return ((p[y][x/32]) >> ((x&31)*2))&3;
}

void Cucumbers::set(const int x, const int y, const int val) {
    printf("set %i %i %i\n", x, y, val);
    p[y][x/32] &= ~(3ll << ((x&31)*2));
    p[y][x/32] |= ((uint64_t)val << ((val&31)*2));
}

void Cucumbers::print() const {
    const size_t BUF_SIZE = sizeof(char)*MAX_Y*MAX_X + MAX_Y + 1;
    char buffer[BUF_SIZE];
    memset(buffer, '.', sizeof(buffer));
#if 0
    for (int j = 0; j < Y; j++) {
        for (int i = 0; i < X; i++) {
            putchar(".>_v"[get(p,i,j)]);
        }
        putchar('\n');
    }
#endif
    buffer[BUF_SIZE-1] = '\0';
    printf("%s\n", buffer);
}

// BETTER, this can be much faster if we pack
// everything in a bit
uint16_t Cucumbers::move() {
    printf("%i %i\n", MAX_X, MAX_Y);
    uint16_t step = 0;
    uint64_t one_moved;
    do {
        one_moved = 0;
        step += 1;
        for (int y = 0; y < MAX_Y; ++y) {
            int preveast = get(MAX_X-1, y) == 1 ? 1 : 0;
            printf("preveast %i %i %i\n", preveast, MAX_X-1, y);
            set(MAX_X, y, get(0, y));
            set(MAX_X+1, y, 3);
            for (uint8_t x = 0; x < (MAX_X+31)/32; ++x) {
                const __uint128_t v = *(__uint128_t*)(&p[y][x]);
                const __uint128_t takenmask = (__int128_t)1 << 64 | 0x5555555555555555l; // FIXME, make global
                __uint128_t taken = v & takenmask;
                uint64_t east =  v & 0xaaaaaaaaaaaaaaaal;
                east = east >> 1;
                east ^= taken;
                taken |= taken << 1;
                const __uint128_t moved = (((__int128_t)east << 2) | preveast) & ~taken;
                uint64_t removed = v ^ (moved >> 2);
                one_moved |= moved;
                p2[y][x] = removed | moved;
                preveast = east >> 62;
            }
        }

        for (uint8_t i = 0; i < (MAX_X+31)/32; ++i) {
            p2[-1][i] = p2[MAX_Y-1][i];
            p2[MAX_Y][i] = p2[0][i];
        }
        for (uint8_t y = 0; y < MAX_Y; ++y) {
        for (uint8_t x = 0; x < (MAX_X+31)/32; ++x) {
            const uint64_t vprev = p2[y-1][x];
            const uint64_t v = p2[y][x];
            uint64_t vnext = p2[y+1][x];
            uint64_t taken = v & 0x5555555555555555l;
            uint64_t takennext = vnext & 0x5555555555555555l;
            uint64_t south =  v & 0xaaaaaaaaaaaaaaaal;
            uint64_t southprev =  vprev & 0xaaaaaaaaaaaaaaaal;
            south |= south >> 1;
            southprev |= southprev >> 1;
            taken |= taken << 1;
            takennext |= takennext << 1;
            const uint64_t movedout = south & ~takennext;
            const uint64_t movedin = southprev & ~taken;
            p[y][x] = (v ^ movedout) | movedin;
            one_moved |= movedin;
        }}
    } while (one_moved);
    return step;
}

bool Cucumbers::read_map(const char* str) {
    size_t it = 0;
    uint8_t i = 0;
    while (str[it] != 0) {
        //printf("%c",str[it]);
        if (str[it] != '.') set(i, _row, (str[it] == 'v')? 3 : 1);
        ++it;
        ++i;
    }
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
    printf("\n");
    const uint16_t answer1 = cucumbers.move();

    file.close();
    cucumbers.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 25 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);

    return 0;
}
