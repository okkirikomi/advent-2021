#include <math.h>
#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const uint16_t INPUT_MAX = 64;
static const uint8_t MAX_STEPS = 250;

typedef struct Launcher {
    void init();
    void destroy();
    bool set_area(const char* str);
    void calc();

    int16_t _highest_point;
    uint16_t _launch_count;

private:
    bool read_num(const char* str, const char stop, int16_t* out);
    bool in_range(const int16_t x, const int16_t y);
    bool launch(int16_t x_vel, int16_t y_vel, int16_t* out_highest);

    int16_t _x_min;
    int16_t _x_max;
    int16_t _y_min;
    int16_t _y_max;
    uint8_t _it;
} Launcher;

void Launcher::init() {
    _x_min = 0;
    _x_max = 0;
    _y_min = 0;
    _y_max = 0;
    _highest_point = 0;
    _launch_count = 0;
}

void Launcher::destroy() {
    // nothing
}

bool Launcher::in_range(const int16_t x, const int16_t y) {
    if (x < _x_min || x > _x_max || y < _y_min || y > _y_max) return false;
    return true;
}

bool Launcher::launch(int16_t x_vel, int16_t y_vel, int16_t* out_highest) {
    int16_t max_h = 0;
    int16_t x = 0, y = 0;
    for (uint8_t n = 0; n < MAX_STEPS; ++n) {
        x += x_vel;
        y += y_vel;

        // we'll never hit the target at that point
        if ((y < _y_min) || (x_vel == 0 && (x < _x_min || x > _x_max))) {
            return false;
        }

        // update the highest point
        if (y > max_h) max_h = y;

        // return if we hit target
        if (in_range(x,y)) {
            *out_highest = max_h;
            return true;
        }

        // else update the velocity and run another step
        x_vel -= ((0 < x_vel) - (x_vel < 0));
        y_vel -= 1;
    }
    return false;
}

static int16_t max(const int16_t a, const int16_t b) {
    return (a > b)? a : b;
}

void Launcher::calc() {
    // BETTER
    // we can do better calculations to avoid calculating
    // impossible shots

    // make sure we don't overshoot immediately
    const int16_t y_max_vel = max(abs(_y_min), abs(_y_max)) + 1;
    const int16_t x_min_vel = int16_t(sqrt(2*_x_min) - 1);

    int16_t highest;
    for (int16_t x_vel = x_min_vel; x_vel <= _x_max; ++x_vel) {
        for (int16_t y_vel = -y_max_vel; y_vel <= y_max_vel; ++y_vel) {
            if (!launch(x_vel, y_vel, &highest)) continue;
            if (highest > _highest_point) _highest_point = highest;
            _launch_count += 1;
        }
    }
}

bool Launcher::read_num(const char* str, const char limit, int16_t* out) {
    bool is_negative = false;
    int16_t value = 0;
    while (str[_it] != 0 && str[_it] != limit) {
        if (ascii_isdigit(str[_it])) {
            const char c = str[_it] - '0';
            value *= 10; // this can overflow
            value += c;
        } else if (str[_it] == '-') {
            is_negative = true;
        }
        ++_it;
    }
    *out = (is_negative)? -value : value;
    return true;
}

// target area: x=20..30, y=-10..-5
bool Launcher::set_area(const char* str) {
    _it = 0;
    if (!read_num(str, '.', &_x_min)) return false;
    if (!read_num(str, ',', &_x_max)) return false;
    if (!read_num(str, '.', &_y_min)) return false;
    if (!read_num(str, '\0', &_y_max)) return false;

    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Launcher launcher;
    launcher.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!launcher.set_area(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    launcher.calc();

    const int16_t answer1 = launcher._highest_point;
    const uint16_t answer2 = launcher._launch_count;

    file.close();
    launcher.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 17 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %u\n", answer2);

    return 0;
}
