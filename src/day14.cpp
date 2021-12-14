#include <stdio.h>
#include <string.h>

#include "file.h"
#include "radix_sort64.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 32;
static const uint8_t ALPHABET_SIZE = 26;
static const uint16_t MAX_PAIRS = ALPHABET_SIZE * ALPHABET_SIZE;

uint16_t hash(const char a, const char b) {
    return (a - 'A') + (b - 'A') * ALPHABET_SIZE;
}

typedef struct Polymer {
    void init();
    void destroy();
    bool add_rule(const char* str);
    bool add_template(const char* str);

    void step_n(const uint8_t n);

    uint64_t score() const;

private:

    uint64_t _letter_count[ALPHABET_SIZE];
    uint64_t _pairs_count[MAX_PAIRS];

    char _rules[MAX_PAIRS];
} Polymer;

void Polymer::init() {
    memset(_letter_count, 0, sizeof(_letter_count));
    memset(_pairs_count, 0, sizeof(_pairs_count));
    memset(_rules, 0, sizeof(_rules));
}

void Polymer::destroy() {
    // nothing
}

uint64_t Polymer::score() const {
    uint64_t sorted_count[ALPHABET_SIZE];
    memcpy(sorted_count, _letter_count, sizeof(_letter_count));
    radix_sort64(sorted_count, ALPHABET_SIZE);

    // highest will always be last position but
    // if we don't have 26 letters, smallest non zero
    // might be further up
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i) {
        if (sorted_count[i] == 0) continue;
        return sorted_count[ALPHABET_SIZE - 1] - sorted_count[i];
    }
    return 0;
}

// Expected sequence:
// BSONBHNSSCFPSFOPHKPK
bool Polymer::add_template(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) {
        const uint8_t ascii_0 = str[it]-65;
        if (ascii_0 >= ALPHABET_SIZE) return false;
        _letter_count[ascii_0] += 1;

        if (str[it+1] == 0) break;
        _pairs_count[hash(str[it], str[it+1])] += 1;
        ++it;
    }
    return true;
}

// they all have the same size
// HH -> K
bool Polymer::add_rule(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) ++it;
    if (it < 6) return false;

    _rules[hash(str[0], str[1])] = str[6];
    return true;
}

void Polymer::step_n(const uint8_t n) {
    // Need a temp array to not mess up with our count when iterating
    uint64_t new_pairs[MAX_PAIRS] = { 0 };
    for (uint8_t step = 0; step < n; ++step) {
        // iterate over all possible pairs
        for(uint16_t i = 0; i < MAX_PAIRS; ++i) {
            // skip if we have none
            const uint64_t count = _pairs_count[i];
            if (count == 0) continue;

            // check if we have a rule for that pair
            const char c = _rules[i];
            if (c == 0) continue;

            // reverse hash to find the two associated chars
            const char a = (i % ALPHABET_SIZE);
            const char b = (i / ALPHABET_SIZE);

            // remove old pair and create two new ones,
            // the new value added between the previouses
            _pairs_count[i] = 0;
            new_pairs[hash(a +'A', c)] += count;
            new_pairs[hash(c, b +'A')] += count;

            // and keep up our total letters count
            _letter_count[c - 'A'] += count;
        }

        // FIXME could be faster with some SIMD shenanigans
        for(uint16_t i = 0; i < MAX_PAIRS; ++i) {
            _pairs_count[i] += new_pairs[i];
            new_pairs[i] = 0;
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
    polymer.step_n(30);
    const uint64_t answer2 = polymer.score();

    file.close();
    polymer.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 14 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %" PRIu64 "\n", answer1);
    printf("Answer 1 = %" PRIu64 "\n", answer2);

    return 0;
}
