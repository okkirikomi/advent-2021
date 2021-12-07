#include <cmath>
#include <stdio.h>

#include "bitset.h"
#include "file.h"
#include "radix.h"
#include "strtoint.h"
#include "timer.h"

static const size_t INPUT_MAX = 4000;
static const uint16_t MAX_CRABS = 1000;

typedef struct Crabs {
    void init();
    bool fill_crab(const char* str);
    uint32_t cost(const int16_t point) const;
    uint32_t cost_two(const int16_t point) const;
    void sort();
    uint16_t median() const;
    void mean(uint16_t* out_ceiling, uint16_t* out_floor) const;

private:
    uint16_t _crabs[MAX_CRABS];
    uint16_t _n_crabs;

} Crabs;

void Crabs::init() {
    _n_crabs = 0;
}

// Part 2's answer is always ceiling or floor of the mean
// BETTER, can we be sure when to use which?
void Crabs::mean(uint16_t* out_ceiling, uint16_t* out_floor) const {
    uint32_t sum = 0; 
    for (size_t i = 0; i < _n_crabs; ++i) {
        sum += _crabs[i];
    }
    *out_floor = sum / _n_crabs;
    *out_ceiling = *out_floor + 1;
}

uint16_t Crabs::median() const {
    const uint16_t half = _n_crabs >> 1;
    if (BIT_CHECK(_n_crabs, 0) == 0) return (_crabs[half] + _crabs[half - 1]) / 2;
    else return _crabs[half];
}

// this is much faster than std::sort here
void Crabs::sort() {
    radix_sort(_crabs, _n_crabs);
}

// We expect a line in the form of:
// 16,1,2,0,4,2,7,1,2,14
bool Crabs::fill_crab(const char* str) {
    size_t it = 0;
    uint16_t value = 0;
    while (str[it] != '\0') {
        if (it > 0 && str[it-1] == ',') {
            _crabs[_n_crabs] = value;
            if ((++_n_crabs) >= MAX_CRABS) return false;
            value = 0;
        }
        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            value *= 10; // this can overflow
            value += c;
        }
        ++it;
    }
    _crabs[_n_crabs] = value;
    _n_crabs++;
    return true;
}

// the overhead of caching the result is slower than
// calculating it each time with the provided input
uint32_t Crabs::cost_two(const int16_t point) const {
    uint32_t cost = 0;
    for (uint16_t i = 0; i < _n_crabs; ++i) {
        const uint16_t n_steps = abs(_crabs[i] - point);
        cost += (n_steps+1) * n_steps / 2;
    }
    return cost;
}

uint32_t Crabs::cost(const int16_t point) const {
    uint32_t cost = 0;
    for (uint16_t i = 0; i < _n_crabs; ++i) {
        cost += abs(_crabs[i] - point);
    }
    return cost;
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

    Crabs crabs;
    crabs.init();

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!crabs.fill_crab(str)) {
            printf("Error parsing input.\n");
            return -1;
        }
    }
    crabs.sort();

    const uint16_t median = crabs.median();
    const uint32_t answer1 = crabs.cost(median);

    uint16_t mean_ceiling, mean_floor;
    crabs.mean(&mean_ceiling, &mean_floor);
    const uint32_t cost_ceiling = crabs.cost_two(mean_floor);
    const uint32_t cost_floor = crabs.cost_two(mean_floor);
    const uint32_t answer2 = cost_floor < cost_ceiling? cost_floor : cost_ceiling;

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 7 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
