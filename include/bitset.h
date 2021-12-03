#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))

#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK_ALL(x,y) (!(~(x) & (y)))
#define BITMASK_CHECK_ANY(x,y) ((x) & (y))

// TODO support more compilers
int popcount32(uint32_t val) {
#if defined(__GNUC__) || defined(__GNUG__)
    return __builtin_popcount(val);
#else
    // use Hamming Weight if no builtin
    val = val - ((val >> 1) & 0x55555555);
    val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
    return (((val + (val >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif  
}

#endif //BITSET_H
