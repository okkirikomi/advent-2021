#include <bitset>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

const uint16_t INPUT_MAX = 128;
const uint16_t MAX_PROCEDURES = 420;
const uint8_t CUBE_SIDE = 101; /// [-50,50]
const uint8_t ZERO_OFFSET = CUBE_SIDE / 2;
const uint32_t N_CUBES = CUBE_SIDE*CUBE_SIDE*CUBE_SIDE;

typedef struct Cube {
    int32_t x1, x2;
    int32_t y1, y2;
    int32_t z1, z2;
    uint64_t size() const {
        return (uint64_t)(x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
    }
    bool collision(const Cube& c) const { 
        if (z2 < c.z1 || c.z2 < z1) return false;
        if (y2 < c.y1 || c.y2 < y1) return false;
        if (x2 < c.x1 || c.x2 < x1) return false;
        return true;
    }
} Cube;

typedef struct Procedure {
    bool on;
    int32_t x[2];
    int32_t y[2];
    int32_t z[2];
} Procedure;

typedef struct Reactor {
    void init();
    void destroy();
    bool read_procedures(const char* str);
    uint32_t part_one() const { return _part_1; }
    uint64_t part_two() const { return _part_2; }
    void step();
    void reboot();

private:
    void add_diff(const Cube& c1, const Cube& c2);
    Procedure _procedures[MAX_PROCEDURES];
    uint16_t _n_procedure;

    Cube _cubes[4000];
    Cube _cubes_buffer[4000];
    uint16_t _n_cubes = 0;
    uint16_t _n_buffer = 0;

    uint32_t _part_1;
    uint64_t _part_2;

} Reactor;

void Reactor::init() {
    _n_procedure = 0;
    _n_cubes = 0;
    _n_buffer = 0;
    _part_1 = 0;
    _part_2 = 0;
}

void Reactor::destroy() {
    // nothing
}

template<class T> 
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T> 
const T& max(const T& a, const T& b)
{
    return (b > a) ? b : a;
}

// add what doesn't collide between the cubes
// as different cubes in our cube list
void Reactor::add_diff(const Cube& c1, const Cube& c2) {
    const int32_t x1 = max(c1.x1, c2.x1);
    const int32_t x2 = min(c1.x2, c2.x2);
    const int32_t y1 = max(c1.y1, c2.y1);
    const int32_t y2 = min(c1.y2, c2.y2);
    const int32_t z1 = max(c1.z1, c2.z1);
    const int32_t z2 = min(c1.z2, c2.z2);

    // all possible cross sections
    if (c1.x1 < x1) {
        _cubes_buffer[_n_buffer].x1 = c1.x1;
        _cubes_buffer[_n_buffer].x2 = (x1-1);
        _cubes_buffer[_n_buffer].y1 = c1.y1;
        _cubes_buffer[_n_buffer].y2 = c1.y2;
        _cubes_buffer[_n_buffer].z1 = z1;
        _cubes_buffer[_n_buffer].z2 = z2;
        _n_buffer +=1;
    }
    if (x2 < c1.x2) {
        _cubes_buffer[_n_buffer].x1 = (x2 + 1);
        _cubes_buffer[_n_buffer].x2 = c1.x2;
        _cubes_buffer[_n_buffer].y1 = c1.y1;
        _cubes_buffer[_n_buffer].y2 = c1.y2;
        _cubes_buffer[_n_buffer].z1 = z1;
        _cubes_buffer[_n_buffer].z2 = z2;
        _n_buffer +=1;
    }
    if (c1.y1 < y1) {
        _cubes_buffer[_n_buffer].x1 = x1;
        _cubes_buffer[_n_buffer].x2 = x2;
        _cubes_buffer[_n_buffer].y1 = c1.y1;
        _cubes_buffer[_n_buffer].y2 = (y1 - 1);
        _cubes_buffer[_n_buffer].z1 = z1;
        _cubes_buffer[_n_buffer].z2 = z2;
        _n_buffer +=1;
    }
    if (y2 < c1.y2) {
        _cubes_buffer[_n_buffer].x1 = x1;
        _cubes_buffer[_n_buffer].x2 = x2;
        _cubes_buffer[_n_buffer].y1 = (y2 + 1);
        _cubes_buffer[_n_buffer].y2 = c1.y2;
        _cubes_buffer[_n_buffer].z1 = z1;
        _cubes_buffer[_n_buffer].z2 = z2;
        _n_buffer +=1;
    }
    if (c1.z1 < z1) {
        _cubes_buffer[_n_buffer].x1 = c1.x1;
        _cubes_buffer[_n_buffer].x2 = c1.x2;
        _cubes_buffer[_n_buffer].y1 = c1.y1;
        _cubes_buffer[_n_buffer].y2 = c1.y2;
        _cubes_buffer[_n_buffer].z1 = c1.z1;
        _cubes_buffer[_n_buffer].z2 = z1 - 1;
        _n_buffer +=1;
    }
    if (z2 < c1.z2) {
        _cubes_buffer[_n_buffer].x1 = c1.x1;
        _cubes_buffer[_n_buffer].x2 = c1.x2;
        _cubes_buffer[_n_buffer].y1 = c1.y1;
        _cubes_buffer[_n_buffer].y2 = c1.y2;
        _cubes_buffer[_n_buffer].z1 = (z2 + 1);
        _cubes_buffer[_n_buffer].z2 = c1.z2;
        _n_buffer +=1;
    }
}

// Here, rather than keeping the whole universe, we just track
// the intented cubes to be turned on using basic 3D collisions
// FIXME, make the cube array safe to increment
void Reactor::reboot() {
    for (uint16_t i = 0; i < _n_procedure; ++i) {
        // create the cube for that procedure
        Cube cube;
        cube.x1 = _procedures[i].x[0];
        cube.x2 = _procedures[i].x[1];
        cube.y1 = _procedures[i].y[0];
        cube.y2 = _procedures[i].y[1];
        cube.z1 = _procedures[i].z[0];
        cube.z2 = _procedures[i].z[1];
        // compare against all our existing cubes
        for (uint16_t j = 0; j < _n_cubes; ++j) {
            // if we have a collision, only add the new parts
            if (_cubes[j].collision(cube)) add_diff(_cubes[j], cube);
            else {
                // else keep it as is
                _cubes_buffer[_n_buffer] = _cubes[j];
                _n_buffer +=1;
            }
        }
        if (_procedures[i].on) {
            _cubes_buffer[_n_buffer] = cube;
            _n_buffer += 1;
        }
        if (_n_buffer == 0) continue;
        // swap with our buffer
        memcpy(_cubes, _cubes_buffer, sizeof(Cube)*_n_buffer);
        _n_cubes = _n_buffer;
        _n_buffer = 0;
    }

    // now count the dots
    for (uint16_t i = 0; i < _n_cubes; ++i) {
        _part_2 += _cubes[i].size();
    }
}

// FIXME, just count the results from boot() instead
// no need to use that bitset
void Reactor::step() {
    std::bitset<N_CUBES> cubes;
    for (uint8_t i = 0; i < 20; ++i) {
        for (int32_t x = _procedures[i].x[0] + ZERO_OFFSET; x <= _procedures[i].x[1] + ZERO_OFFSET; ++x) {
        for (int32_t y = _procedures[i].y[0] + ZERO_OFFSET; y <= _procedures[i].y[1] + ZERO_OFFSET; ++y) {
            const int32_t xy = x + y * CUBE_SIDE;
        for (int32_t z = _procedures[i].z[0] + ZERO_OFFSET; z <= _procedures[i].z[1] + ZERO_OFFSET; ++z) {
            const uint32_t index = xy + z * CUBE_SIDE * CUBE_SIDE;
            cubes.set(index, _procedures[i].on);
        }}}
    }
    _part_1 = cubes.count();
}

static uint32_t get_int(const char* str, uint8_t& it) {
    int negative = 0;
    int32_t ret = 0;

    if (str[it] == '-') {
        negative = 1;
        it += 1;
    } else if (str[it] == '+') {
        it += 1;
    }

    while (str[it] != 0) {
        if (ascii_isdigit(str[it])) {
            ret *= 10; // this can overflow
            ret += (str[it] - '0');
        } else break;
        it += 1;
    }

    if (negative) ret = -ret;
    return ret;
}

bool Reactor::read_procedures(const char* str) {
    if (_n_procedure == MAX_PROCEDURES) return false;

    // check on/off
    if (str[0] != 'o') return false;
    if (str[1] == 'n') _procedures[_n_procedure].on = true;
    else if (str[1] == 'f') _procedures[_n_procedure].on = false;
    else return false;

    // skip to x
    uint8_t it = 2;
    while (str[it] != 0) {
        if (str[it] == 'x') break;
        it += 1;
    }
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].x[0] = get_int(str, it);
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].x[1] = get_int(str, it);

    // skip to y
    while (str[it] != 0) {
        if (str[it] == 'y') break;
        it += 1;
    }
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].y[0] = get_int(str, it);
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].y[1] = get_int(str, it);

    // skip to z
    while (str[it] != 0) {
        if (str[it] == 'z') break;
        it += 1;
    }
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].z[0] = get_int(str, it);
    it += 2; // FIXME, unsafe
    _procedures[_n_procedure].z[1] = get_int(str, it);

    _n_procedure += 1;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Reactor reactor;
    reactor.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX) > 1) {
        if (!reactor.read_procedures(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    reactor.step();
    const uint32_t answer1 = reactor.part_one();
    reactor.reboot();
    const uint64_t answer2 = reactor.part_two();

    file.close();
    reactor.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 22 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %lu\n", answer2);

    return 0;
}
