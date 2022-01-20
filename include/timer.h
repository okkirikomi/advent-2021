#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>
#include <stdint.h>


#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint64_t cycle_start;

void cycle_begin() {
    cycle_start = __rdtsc();
}

uint64_t cycle_stop() {
    return cycle_start - __rdtsc();
}

// TODO, support Windows with QueryPerformanceCounter
#ifdef _WIN32
#error Timer.h not implemented for windows.
#else

#include <time.h>

static struct timespec start, stop;

static void timer_start() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
}

// this is µs
static uint64_t timer_stop() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    return (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;
}

#endif // _WIN32



#endif // TIMER_H
