#include <stdio.h>
#include <string.h>

#include "file.h"
#include "stack.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint16_t Y_MAX = 100;
static const uint16_t X_MAX = 100;
//static const uint8_t Y_MAX = 10;
//static const uint8_t X_MAX = 10;
static const uint16_t MAX_CHITONS = Y_MAX * X_MAX;

typedef struct Pos {
    Pos(const uint8_t in_x, const uint8_t in_y) : x(in_x), y(in_y) {}
    Pos() {}
    uint8_t x,y;
    bool operator== (const Pos& p) const {
        return x == p.x && y == p.y;
    }
} Pos;

uint16_t hash(const uint8_t a, const uint8_t b) {
    return a + b * X_MAX;
}

typedef struct Cave {
    void init();
    void destroy();
    bool add_row(const char* str);
    uint16_t lowest_risk();

private:
    void visit(const Pos& new_pos, const uint16_t hash_pos);

    uint16_t _cost_for[MAX_CHITONS];
    uint8_t _chitons[Y_MAX][X_MAX]; // FIXME, use hash based index instead
    uint8_t _y;
    Stack<Pos> _to_visit;

} Cave;

void Cave::init() {
    _y = 0;
    memset(_cost_for, 0, sizeof(_cost_for));
    _to_visit.init(128);
}

void Cave::destroy() {
    // nothing
}

void Cave::visit(const Pos& to, const uint16_t from) {
    const uint16_t hash_to = hash(to.x, to.y);

    // cost for going to that new pos
    const uint16_t new_cost = _cost_for[from] + _chitons[to.y][to.x];

    //printf("    -> %u,%u %u < %u?\n", to.x, to.y, new_cost, _cost_for[hash_to]);

    // visit if it's better than what we have for that pos already
    if (_cost_for[hash_to] == 0 || new_cost < _cost_for[hash_to]) {
        
        _cost_for[hash_to] = new_cost;

        // FIXME, can we queue better to make sure the first time
        // we reach the goal it's the best cost already?
        _to_visit.push(to);
    }
}

uint16_t Cave::lowest_risk() {
    uint16_t risk = 0;
    //const Pos goal(X_MAX - 1, Y_MAX - 1);

    _to_visit.push(Pos(0,0));

    Pos pos;
    while (_to_visit.pop(pos)) {
        //printf("\nVisiting %u,%u \n",pos.x, pos.y);

        const uint16_t hash_pos = hash(pos.x, pos.y);
        // try the 4 neighbors
        if (pos.x != 0) visit(Pos(pos.x-1, pos.y), hash_pos);
        if (pos.y != 0) visit(Pos(pos.x, pos.y-1), hash_pos);
        if (pos.x != X_MAX -1) visit(Pos(pos.x+1, pos.y), hash_pos);
        if (pos.y != Y_MAX -1) visit(Pos(pos.x, pos.y+1), hash_pos);
    }

    return _cost_for[hash(X_MAX - 1, Y_MAX - 1)];
}

bool Cave::add_row(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) {
        if (it == X_MAX || _y == Y_MAX) return false;
        _chitons[_y][it] = str[it] - '0';
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

    const uint16_t answer1 = cave.lowest_risk();
    const uint32_t answer2 = 0;

    file.close();
    cave.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 14 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);
    printf("Answer 1 = %u\n", answer2);

    return 0;
}
