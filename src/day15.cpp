#include <stdio.h>
#include <string.h>

#include "file.h"
#include "priority_queue.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
//static const uint16_t Y_MAX = 100;
//static const uint16_t X_MAX = 100;
static const uint8_t Y_MAX = 10;
static const uint8_t X_MAX = 10;
static const uint16_t X_MAX_REAL = X_MAX * 25;
static const uint16_t Y_MAX_REAL = Y_MAX * 25;
static const uint16_t MAX_CHITONS = Y_MAX * X_MAX;

typedef struct Pos {
    Pos(const uint16_t in_x, const uint16_t in_y) : x(in_x), y(in_y) {}
    Pos() {}
    uint16_t x,y;
    bool operator== (const Pos& p) const {
        return x == p.x && y == p.y;
    }
} Pos;

uint16_t hash(const uint16_t a, const uint16_t b) {
    return a + b * X_MAX;
}

typedef struct Cave {
    void init();
    void destroy();
    bool add_row(const char* str);
    uint16_t lowest_risk(const uint16_t max_x, const uint16_t max_y);

    uint16_t _max_x;
    uint16_t _max_y;

private:
    void visit(const uint16_t to_x, const uint16_t to_y, const uint16_t hash_from);
    uint8_t risk_at(const uint16_t a, const uint16_t b);

    uint16_t _cost_for[MAX_CHITONS];
    uint8_t _chitons[MAX_CHITONS];
    uint8_t _y;
    PriorityQueue<uint16_t> _to_visit;

} Cave;

void Cave::init() {
    _max_x = X_MAX; // FIXME
    _max_y = Y_MAX; // FIXME
    _y = 0;
    memset(_cost_for, 0, sizeof(_cost_for));
    _to_visit.init(128);
}

void Cave::destroy() {
    // nothing
}

// TODO, check if using a grid 25x larger is faster than
// calculating risk over and over
uint8_t risk_at(const uint16_t x, const uint16_t y) {
    // loop around to 25 times X_MAX, Y_MAX
    // increasing risk +1 and looping at 9 -> 1

    return 0;
}

void Cave::visit(const uint16_t to_x, const uint16_t to_y, const uint16_t cost_from) {
    const uint16_t hash_to = hash(to_x, to_y);

    // cost for going to that new pos
    const uint16_t new_cost =  cost_from + _chitons[hash_to];

    printf("    -> %u,%u %u < %u?\n", to_x, to_y, new_cost, _cost_for[hash_to]);

    // visit if it's better than what we have for that pos already
    if (_cost_for[hash_to] == 0 || new_cost < _cost_for[hash_to]) {
        _cost_for[hash_to] = new_cost;

        printf("Pushing %u,%u cost %u\n", to_x,to_y, new_cost);
        // FIXME, the priority queue is busted
        _to_visit.push(hash_to, new_cost);
    }
}

uint16_t Cave::lowest_risk(const uint16_t max_x, const uint16_t max_y) {
    _to_visit.push(0, 0);
    uint16_t goal = hash(max_x - 1, max_y - 1);

    uint16_t hash_pos;
    while (_to_visit.pop(hash_pos)) {
        const uint16_t x = hash_pos % X_MAX;
        const uint16_t y = hash_pos / X_MAX;
        const uint16_t cost = _cost_for[hash_pos];

        printf("\nVisiting %u,%u \n",x, y);

        // try the 4 neighbors
        if (x != 0) visit(x-1, y, cost);
        if (y != 0) visit(x, y-1, cost);
        if (x != X_MAX -1) visit(x+1, y, cost);
        if (y != Y_MAX -1) visit(x, y+1, cost);

        //if (hash_pos == goal) break;

        _to_visit.print();

    }

    //for (uint16_t i = 0 ; i < MAX_CHITONS; ++i)
    //    printf("%u: %u\n", i, _cost_for[i]);

    return _cost_for[hash(X_MAX - 1, Y_MAX - 1)];
}

bool Cave::add_row(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) {
        if (it == X_MAX || _y == Y_MAX) return false;
        _chitons[hash(it, _y)] = str[it] - '0';
        ++it;
    }
    ++_y;
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Cave cave;
    cave.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!cave.add_row(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    const uint16_t answer1 = cave.lowest_risk(X_MAX, Y_MAX);
    const uint32_t answer2 = 0;

    file.close();
    cave.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 15 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);
    printf("Answer 1 = %u\n", answer2);

    return 0;
}
