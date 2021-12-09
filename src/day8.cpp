#include <cmath>
#include <stdio.h>
#include <unordered_map>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint8_t MAX_PATTERNS = 10;
static const uint8_t N_DIGITS = 10;
static const uint8_t N_OUTPUT = 4;
static const uint8_t VAL_LENGTH = 7 + 1; // extra for \0

const uint8_t segments_for_digit[N_DIGITS] = { 6, 2, 5, 5, 4, 5, 6, 3, 7, 6 };

// BETTER, use a better data structure than std
typedef std::unordered_map<char, int> char_count;

typedef struct Segments {
    bool process_input(const char* str);
    void init();
    uint16_t unique_segment() const { return _n_unique_segments; }
    uint32_t sum_outputs() const { return _sum_outputs; }

private:
    void check_unique_segments(const uint8_t size);
    void solve(const char output[N_OUTPUT][VAL_LENGTH]);
    void set_key(const char patterns[MAX_PATTERNS][VAL_LENGTH]);
    uint16_t _n_unique_segments;
    uint32_t _sum_outputs;
    char_count _key;

} Segments;

void Segments::init() {
    _n_unique_segments = 0;
    _sum_outputs = 0;
    _key.reserve(7);
}

static int ascii_is_signal(const int c) {
    return ((c >= 'a' && c <= 'g')? 1 : 0);
}

void Segments::set_key(const char patterns[MAX_PATTERNS][VAL_LENGTH]) {
    _key = {{'a', 0}, {'b', 0}, {'c', 0}, {'d', 0}, {'e', 0}, {'f', 0}, {'g', 0}};
    for (uint8_t i = 0; i < MAX_PATTERNS; ++i) {
        uint8_t it = 0;
        while (patterns[i][it] != '\0') {
            _key[patterns[i][it]] += 1;
            ++it;
        }
    }
}

// Not 100% sure why this works yet
void Segments::solve(const char output[N_OUTPUT][VAL_LENGTH]) {
    for (uint8_t i = 1; i < 5; ++i) {
        uint8_t str_it = 0;
        int total = 0;
        while (output[i-1][str_it] != '\0') {
            total += _key[output[i-1][str_it]];
            ++str_it;
        }
        int d;
        switch (total) {
            default:
            case 42: d = 0; break;
            case 17: d = 1; break;
            case 34: d = 2; break;
            case 39: d = 3; break;
            case 30: d = 4; break;
            case 37: d = 5; break;
            case 41: d = 6; break;
            case 25: d = 7; break;
            case 49: d = 8; break;
            case 45: d = 9; break;
        }
        _sum_outputs += d * pow(10, (4-i));
    }
}

void Segments::check_unique_segments(const uint8_t size) {
    if (size == segments_for_digit[1] || size == segments_for_digit[4] ||
        size == segments_for_digit[7] || size == segments_for_digit[8]) {
        ++_n_unique_segments;
    }
}

// We expect a line in the form of:
// acedgfb cdfbe gcdfa fbcad dab cefabd cdfgeb eafb cagedb ab | cdfeb fcadb cdfeb cdbaf
bool Segments::process_input(const char* str) {
    size_t str_it = 0;
    uint8_t it_pattern = 0;
    uint8_t it_fill = 0;
    char patterns[MAX_PATTERNS][VAL_LENGTH];

    // fill the 10 signal patterns
    while (str[str_it] != '\0') {
        if (ascii_is_signal(str[str_it])) {
            patterns[it_pattern][it_fill] = str[str_it];
            if ((++it_fill) >= VAL_LENGTH) return false;
        } else if (str[str_it] == ' ' && str[str_it-1] != '|') {
            patterns[it_pattern][it_fill] = '\0';
            ++it_pattern;
            if (str[str_it+1] != '|' && it_pattern >= MAX_PATTERNS) return false;
            it_fill = 0;
        } else if (str[str_it] == '|') {
            ++str_it;
            break;
        }
        ++str_it;
    }
    set_key(patterns);

    char output[N_OUTPUT][VAL_LENGTH];
    it_pattern = 0;
    it_fill = 0;

    // fill the 4 output values
    while (str[str_it] != '\0') {
        if (ascii_is_signal(str[str_it])) {
            output[it_pattern][it_fill] = str[str_it];
            if ((++it_fill) >= VAL_LENGTH) return false;
        } else if (str[str_it] == ' ' && it_fill > 0) {
            output[it_pattern][it_fill] = '\0';
            check_unique_segments(it_fill);
            if(++it_pattern >= N_OUTPUT) return false;
            it_fill = 0;
        }
        ++str_it;
    }
    output[it_pattern][it_fill] = '\0';
    check_unique_segments(it_fill);
    solve(output);
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

    Segments patterns;
    patterns.init();

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!patterns.process_input(str)) {
            printf("error with input.\n");
            return -1;
        }
    }

    const uint16_t answer1 = patterns.unique_segment();
    const uint32_t answer2 = patterns.sum_outputs();

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 8 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
