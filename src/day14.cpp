#include <array>
#include <unordered_map>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "radix_sort64.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 32;
static const uint8_t ALPHABET_SIZE = 26;
static const uint64_t MAX_POLYMER_SIZE = UINT32_MAX;

struct equal_to {  
    bool operator()(const char str_a[2], const char str_b[2]) const  
    {
        return str_a[0] == str_b[0] && str_a[1] == str_b[1];
    }  
};

struct hash_func {
    int operator()(const std::array<char, 2> str) const
    {
        return str[0] + str[1] * ALPHABET_SIZE;
    }
};

typedef struct Polymer {
    void init();
    void destroy();
    bool add_rule(const char* str);
    bool add_template(const char* str);

    void step_n(const uint8_t n);

    uint64_t score();

private:
    void shit_insert(uint64_t at, char c);
    uint64_t _letter_count[ALPHABET_SIZE];
    char* _string;
    uint64_t _it;

    // FIXME don't use std
    std::unordered_map<std::array<char, 2>, char, hash_func> _rules;

} Polymer;

void Polymer::init() {
    memset(_letter_count, 0, sizeof(_letter_count));
    _string = (char*) malloc(sizeof(char)*MAX_POLYMER_SIZE);
    _it = 0;
    _rules.reserve(100);
}

void Polymer::destroy() {
    free(_string);
}

uint64_t Polymer::score() {
    radix_sort64(_letter_count, ALPHABET_SIZE);

    // highest will always be last position but
    // if we don't have 26 letters, smallest non zero
    // might be further up
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i) {
        if (_letter_count[i] == 0) continue;
        return _letter_count[ALPHABET_SIZE - 1] - _letter_count[i];
    }
    return 0;
}

bool Polymer::add_template(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) {
        _string[it] = str[it];
        _letter_count[str[it]-65] += 1; // FIXME make safe
        ++it;
    }
    _it = it;
    return true;
}

// they all have the same size
// HH -> K
bool Polymer::add_rule(const char* str) {
    std::array<char,2> base;
    char to;
    uint8_t it = 0;

    while (str[it] != 0) ++it;
    if (it < 6) return false;

    base[0] = str[0];
    base[1] = str[1];
    to = str[6];

    printf("%c%c -> %c\n", base[0], base[1], to);

    _rules.emplace(base, to);
    return true;
}

// FIXME, implement string as a rope and don't do this garbage
void Polymer::shit_insert(uint64_t at, char c) {
    //printf("%lu %c\n", at, c);
    char tmp = _string[at];
    _it += 1;
    while (at < _it) {
        tmp = _string[at];
        _string[at] = c;
        c = tmp;
        ++at;
    }
}

void Polymer::step_n(const uint8_t n) {
    std::array<char, 2> key;
    for (uint8_t step = 0; step < n; ++step) {
        printf("STEP %u %lu\n", step, _it);
        for (uint64_t it = 0; it < _it - 1; ++it) {
            key[0] = _string[it];
            key[1] = _string[it+1];
            auto found = _rules.find(key);
            if (found == _rules.end()) continue;

            shit_insert(it+1, found->second);
            _letter_count[found->second-65] += 1;
            ++it;
        }
    }
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Polymer polymer;
    polymer.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    file.readline(str, INPUT_MAX);
    polymer.add_template(str);

    // skip empty line
    file.readline(str, INPUT_MAX);

    while (file.readline(str, INPUT_MAX)) {
        if (!polymer.add_rule(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    polymer.step_n(10);
    const uint64_t answer1 = polymer.score();

    // need 40 steps total
    //polymer.step_n(30);
    const uint64_t answer2 = polymer.score();


    file.close();
    polymer.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 13 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %" PRIu64 "\n", answer1);
    printf("Answer 1 = %" PRIu64 "\n", answer2);

    return 0;
}
