#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t INPUT_MAX = 128;
static const size_t MAX_PATTERNS = 210;
static const size_t PATTERN_LENGTH = 64;
//static const size_t OUTPUT_LENGTH = 32;
static const size_t N_DIGITS = 10;
static const size_t N_OUTPUT = 4;
static const size_t N_SEGMENTS = 7 + 1; // extra for \0

const uint8_t segments_digit[N_DIGITS] = { 6, 2, 5, 5, 4, 5, 6, 3, 7, 6 };

typedef struct Signal {
    char string[N_SEGMENTS];
    uint8_t len;
} Signal;

// FIXME this name is garbage
typedef struct Signal_patterns {
    bool input(const char* str);
    void init();
    void print_all();
    uint32_t unique_segment_counts() const ;
    void decode();

private:
    Signal* next_fill();
    // we don't need to keep all patterns, just the current
    Signal _patterns[MAX_PATTERNS][N_DIGITS];
    Signal _output[MAX_PATTERNS][N_OUTPUT];
    uint8_t _n_patterns;
    uint8_t _n_digit;
    bool _input_patterns;

} Signal_patterns;

void Signal_patterns::init() {
    _input_patterns = true;
    _n_patterns = 0;
    _n_digit = 0;
}

void Signal_patterns::print_all() {
    for (size_t i = 0; i < _n_patterns; ++i) {
        for (size_t j = 0; j < N_DIGITS; ++j)
            printf("|%s| ", _patterns[i][j].string);
        printf("%zu\n", i);
    }
    printf("|\n");
    for (size_t i = 0; i < _n_patterns; ++i) {
        for (size_t j = 0; j < N_OUTPUT; ++j)
            printf("|%s| ", _output[i][j].string);
        printf("\n");
    }
}

// FIXME, count them as we parse the input
uint32_t Signal_patterns::unique_segment_counts() const {
    uint32_t count = 0;
    for (size_t i = 0; i < _n_patterns; ++i)
    for (size_t j = 0; j < N_OUTPUT; ++j) {
        const uint8_t& len = _output[i][j].len;
        if (len == segments_digit[1] || len == segments_digit[4]
         || len == segments_digit[7] || len == segments_digit[8]) {
            ++count;
            //printf("%s\n",_output[i][j]);
        }
    }
    return count;
}

Signal* Signal_patterns::next_fill() {
    //printf("next_fill %i %u %u\n", _input_patterns, _n_patterns, _n_digit);
    if (_input_patterns)  {
        return &_patterns[_n_patterns][_n_digit];
    } else {
        //if (_n_digit >= N_OUTPUT) return NULL;
        return &_output[_n_patterns][_n_digit];
    }
}

inline int ascii_is_signal(const int c) {
    return ((c >= 'a' && c <= 'g')? 1 : 0);
}

void Signal_patterns::decode() {

}

// We expect a line in the form of:
// acedgfb cdfbe gcdfa fbcad dab cefabd cdfgeb eafb cagedb ab | cdfeb fcadb cdfeb cdbaf
bool Signal_patterns::input(const char* str) {
    //printf("INPUT: %s\n", str);
    size_t it = 0;
    size_t it_pattern = 0;
    Signal* to_fill = next_fill();
    _n_digit = 1;
    _input_patterns = true;
    while (str[it] != '\0') {
        if (ascii_is_signal(str[it])) {
            to_fill->string[it_pattern] = str[it];
            if ((++it_pattern) >= N_SEGMENTS) return false;
        } else if (str[it] == ' ' && str[it-1] != '|') {
            to_fill->string[it_pattern] = '\0';
            to_fill->len = it_pattern;
            //printf("FILLING |%s|\n", to_fill->string);
            to_fill = next_fill();
            if((++_n_digit) == N_DIGITS) _n_digit = 0;
            it_pattern = 0;
        } else if (str[it] == '|') {
            //printf("SWITCH\n");
            _input_patterns = false;
            _n_digit = 0;
            to_fill = next_fill();
            _n_digit = 1;
        }
        ++it;
    }
    to_fill->string[it_pattern] = '\0';
    //printf("FILLING |%s|\n", to_fill->string);
    to_fill->len = it_pattern;
    _n_digit = 0;
    // FIXME proper check
    if ((++_n_patterns) >= MAX_PATTERNS) return false;
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

    Signal_patterns patterns;
    patterns.init();

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!patterns.input(str)) {
            printf("error with input.\n");
            return -1;
        }
    }
    //patterns.print_all();

    const uint32_t answer1 = patterns.unique_segment_counts();
    const uint32_t answer2 = 0;

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 8 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
