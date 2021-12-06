#include <assert.h>
#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t INPUT_MAX = 724;
static const size_t GESTATION_LENGTH = 8;

typedef struct Gestation {
    void init();
    bool fill_fish(const char* str);
    void iter(const uint days);
    int64_t fish_count() const;

private:
    // track of how many fishes are at a particular day
    int64_t _days[GESTATION_LENGTH + 1];
} Gestation;

void Gestation::init() {
    for (size_t i = 0; i < GESTATION_LENGTH+1; ++i) {
        _days[i] = 0;
    }
}

void Gestation::iter(const uint days) {
    for (size_t day = 0; day < days; ++day) {
        int64_t tmp = _days[0]; // these ones will give birth
        // otherwise, just down their days
        for (size_t i = 0; i < GESTATION_LENGTH; ++i) {
            _days[i] = _days[i+1];
        }
        _days[6] += tmp; // after giving birth, reset to 6
        _days[GESTATION_LENGTH] = tmp; // new fishes start at 8
    }
}

int64_t Gestation::fish_count() const {
    int64_t count = 0;
    for (size_t i = 0; i < GESTATION_LENGTH+1; ++i) {
        count += _days[i];
    }
    return count;
}

// We expect a line in the form of:
// 3,4,3,1,2
bool Gestation::fill_fish(const char* str) {
    size_t it = 0;
    uint8_t value = 0;
    while (str[it] != '\0') {
        if (it > 0 && str[it-1] == ',') {
            _days[value] += 1;
            value = 0;
        }
        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            value *= 10;
            if (value >= GESTATION_LENGTH) return false;
            value += c;
        }
        ++it;
    }
    _days[value] += 1;
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

    Gestation gestation;
    gestation.init();

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!gestation.fill_fish(str)) {
            printf("Input parsing error.\n");
        }
    }

    gestation.iter(80);
    const uint64_t answer1 = gestation.fish_count();
    gestation.iter(256-80);
    const uint64_t answer2 = gestation.fish_count();

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 5 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %" PRIu64"\n", answer1);
    printf("Answer 1 = %" PRIu64"\n", answer2);

    return 0;
}
