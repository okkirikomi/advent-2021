#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>
#include <stdint.h>

// FIXME, support Windows with QueryPerformanceCounter
#ifdef _WIN32
#error Not defined for windows yet.
#else

#include <time.h>

static struct timespec start, stop;

static void timer_start() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
}

static uint64_t timer_stop() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    return (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;
}

#endif // _WIN32

#endif // TIMER_H
