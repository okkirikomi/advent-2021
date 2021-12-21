#include <bitset>
#include <stdio.h>

#include "bitset.h"
#include "file.h"
#include "timer.h"

const uint16_t INPUT_MAX = 513;
const uint16_t ALGO_SIZE = 512;

// We preallocate everything, so if you intend to step more, change this
const uint16_t STEP_ADD = 2;
const uint16_t MAX_STEP = 50;
const uint16_t BASE_PICTURE_SIDE = 100;
const uint16_t PADDING = 4 + (STEP_ADD * MAX_STEP);
const uint16_t PICTURE_SIDE = BASE_PICTURE_SIDE + PADDING;
const uint16_t PICTURE_SIZE = PICTURE_SIDE * PICTURE_SIDE;
const uint16_t INPUT_START = PADDING / 2 + (PADDING / 2 * PICTURE_SIDE);

// BETTER, Can we keep an array of all the 9-bit binary numbers
// we have in picture instead of ALL pixels?

typedef struct Enhancer {
    void init();
    void destroy();
    bool read_algorithm(const char* str);
    bool read_picture(const char* str);

    void enhance_n(const uint8_t n);
    size_t pixels_on() const;

    size_t _answer_1;

private:
    uint16_t algo_index(const uint16_t input_i);

    std::bitset<PICTURE_SIZE> _picture;
    std::bitset<PICTURE_SIZE> _buffer;
    std::bitset<ALGO_SIZE> _algorithm;

    uint16_t _algo_read;
    uint16_t _picture_line;

    uint32_t _picture_start;
    uint8_t _infinite_value;

} Enhancer;

void Enhancer::init() {
    _algo_read = 0;
    _picture_line = 0;
    _picture_start = 0;
    _infinite_value = 0;
    _answer_1 = 0;
}

void Enhancer::destroy() {
    // nothing
}

size_t Enhancer::pixels_on() const {
    return _picture.count();
}

bool Enhancer::read_picture(const char* str) {
    uint8_t it = 0;
    const uint16_t line_start = INPUT_START + _picture_line * PICTURE_SIDE;
    while (str[it] != 0) {
        if (str[it] == '#') {
            if (line_start + it == PICTURE_SIZE) return false;
            _picture.set(line_start + it, 1);
        }
        ++it;
    }
    _picture_line += 1;
    return true;
}

bool Enhancer::read_algorithm(const char* str) {
    uint16_t it = 0;
    while (str[it] != 0) {
        if (str[it] == '#') {
            if (_algo_read + it == ALGO_SIZE) return false;
            _algorithm.set(_algo_read + it, 1);
        }
        ++it;
    }
    _algo_read += it;
    return true;
}

static inline uint16_t picture_index(const uint16_t x, const uint16_t y) { 
    return x + y * PICTURE_SIDE;
}

/*
    Each pixel of the output image is determined by looking at a 3x3 square
    of pixels centered on the corresponding input image pixel.

    The picture is considered infinite, so anything not in our bounds is 0.
*/
uint16_t Enhancer::algo_index(const uint16_t input_i) {
    uint16_t binary_index = 0;
    const uint16_t x = input_i % PICTURE_SIDE;
    const uint16_t y = input_i / PICTURE_SIDE;

    const bool first_row = (y == 0);
    const bool last_row = (y == (PICTURE_SIDE-1));
    const bool first_column = (x == 0);
    const bool last_column = (x == (PICTURE_SIDE-1));

    // we could do a complicated loop ... or just unroll it, it's only 9 points

    binary_index += (first_column || first_row)?  _infinite_value : _picture[picture_index(x - 1, y - 1)];
    binary_index <<= 1;
    binary_index += (first_row)? _infinite_value : _picture[picture_index(x, y - 1)];
    binary_index <<= 1;
    binary_index += (last_column || first_row)? _infinite_value : _picture[picture_index(x + 1, y - 1)];
    binary_index <<= 1;

    binary_index += (first_column)? _infinite_value : _picture[picture_index(x - 1, y)];
    binary_index <<= 1;
    binary_index += _picture[picture_index(x, y)];
    binary_index <<= 1;
    binary_index += (last_column)? _infinite_value : _picture[picture_index(x + 1, y)];
    binary_index <<= 1;

    binary_index += (first_column || last_row)? _infinite_value : _picture[picture_index(x - 1, y + 1)];
    binary_index <<= 1;
    binary_index += (last_row)? _infinite_value : _picture[picture_index(x, y + 1)];
    binary_index <<= 1;
    binary_index += (last_column || last_row)? _infinite_value : _picture[picture_index(x + 1, y + 1)];

    return binary_index;
}

void Enhancer::enhance_n(const uint8_t n) {
    _infinite_value = 0;
    // BETTER, don't scan everything before having extended into it
    for (uint8_t step = 0; step < n; ++step) {
        for (uint16_t i = 0; i < PICTURE_SIZE; ++i) {
            _buffer[i] = _algorithm[algo_index(i)];
        }
        std::swap(_buffer, _picture);
        _buffer.reset();
        _infinite_value = !_infinite_value;

        // save this answer for part 1
        if (step == 1) _answer_1 = pixels_on();
    }
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Enhancer enhancer;
    enhancer.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX) > 1) {
        if (!enhancer.read_algorithm(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }
    while (file.readline(str, INPUT_MAX)) {
        if (!enhancer.read_picture(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    enhancer.enhance_n(50);

    const size_t answer1 = enhancer._answer_1;
    const size_t answer2 = enhancer.pixels_on();

    file.close();
    enhancer.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 20 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %zu\n", answer1);
    printf("Answer 2 = %zu\n", answer2);

    return 0;
}
