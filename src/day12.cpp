#include <stdio.h>

#include "file.h"
#include "stack.h"
#include "strtoint.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 16;
static const uint8_t CONNECTIONS_MAX = 16;

// enough indexes for 2 wide alphabet strings + start, end, null
// BETTER, check if using the smallest possible array
// and having a function to map to an index is faster
static const uint16_t NODES_MAX = 26 * 26 + 3;
static const uint16_t START = NODES_MAX - 1;
static const uint16_t END = NODES_MAX - 2;
static const uint16_t NONE = NODES_MAX - 3;

bool ascii_islarge(const uint8_t c) {
    return ((c >= 'A' && c <= 'Z') ? 1 : 0);
}

// zero based ascii alphabet index
uint16_t str_to_node(const char str[2]) {
    return (str[0] - 65) + (str[1] - 65) * 26;
}

typedef struct Path {
    Path() : visited{NONE}, n_visited(0), small_twice(false) {};

    uint16_t visited[CONNECTIONS_MAX];
    uint8_t n_visited;
    bool small_twice;

    bool add_visited(const uint16_t add) {
        if (n_visited == CONNECTIONS_MAX) return false;
        visited[n_visited] = add;
        n_visited += 1;
        return true;
    }

    // small enough that we can just use an array
    // no need for an hash table
    bool is_visited(const uint16_t node) const {
        for (uint16_t i = 0; i < n_visited; ++i) {
            if (visited[i] == node) return true;
        }
        return false;
    }

    uint16_t last() const {
        assert(n_visited > 0);
        return visited[n_visited-1];
    }
} Path;

typedef struct Map {
    Map() : _connections{NONE}, _connection_count{0} { }
    void init();
    void destroy();

    bool add_node(const char* str);
    bool calc_paths();

    uint16_t simple_visit_count;
    uint32_t part_two_count;

private:
    bool add_connection(const uint16_t from, const uint16_t to);
    bool _big_caves[NODES_MAX];
    uint16_t _connections[NODES_MAX][CONNECTIONS_MAX];
    uint8_t _connection_count[NODES_MAX];
    Stack<Path> _to_continue;

} Map;

void Map::init() {
    _to_continue.init(32);
    simple_visit_count = 0;
    part_two_count = 0;
}

void Map::destroy() {
    _to_continue.destroy();
}

bool Map::add_connection(const uint16_t from, const uint16_t to) {
    uint8_t& max = _connection_count[from];
    if (max == CONNECTIONS_MAX) return false;

    _connections[from][max] = to;
    max += 1;
    return true;
}

// BETTER, find a way to avoid all these struct Path copies
bool Map::calc_paths() {
    // all paths begin at START
    Path path;
    path.visited[0] = START;
    path.n_visited = 1;

    for (uint16_t i = 0; i < _connection_count[START]; ++i) {
        Path new_path = path;
        new_path.add_visited(_connections[START][i]);
        _to_continue.push(new_path);
    }

    // continue all possible paths
    while (_to_continue.pop(path)) {
        const uint32_t current_node = path.last();
        const bool small_twice = path.small_twice;

        // try to visit all connected nodes
        const uint8_t max = _connection_count[current_node];
        for (uint16_t i = 0; i < max; ++i) {
            const uint32_t to_visit = _connections[current_node][i];
            if (to_visit == END) {
                part_two_count += 1;
                if (small_twice == false) simple_visit_count += 1;
                continue;
            }

            // each path is allowed to visit a small cave two times ONCE
            if (!_big_caves[to_visit] && path.is_visited(to_visit)) {
                if (small_twice == false) {
                    Path new_path = path;
                    if(!new_path.add_visited(to_visit)) return false;
                    new_path.small_twice = true;
                    _to_continue.push(new_path);
                }
                continue;
            }
            Path new_path = path;
            if(!new_path.add_visited(to_visit)) return false;
            _to_continue.push(new_path);
        }
    }
    return true;
}

bool Map::add_node(const char* str) {
    uint8_t it = 0;
    char node_str[2];
    uint16_t node_a, node_b;

    // TODO make this safe
    if (str[2] == '-') {
        const bool is_large = ascii_islarge(str[0]);
        node_str[0] = (is_large)? str[0] : str[0] - 32;
        node_str[1] = (is_large)? str[1] : str[1] - 32;
        node_a = str_to_node(node_str);
        _big_caves[node_a] = is_large;
        it += 3;
    } else if (str[2] == 'd') {
        node_a = END;
        it += 4;
    } else if (str[2] == 'a') {
        node_a = START;
        it += 6;
    } else {
        return false;
    }
    const uint8_t it_hyphen = it;
    while (str[it] != '\0') ++it;

    const uint8_t char_left =  it - it_hyphen;
    if (char_left == 2) {
        const bool is_large = ascii_islarge(str[it_hyphen]);
        node_str[0] = (is_large)? str[it_hyphen] : str[it_hyphen] - 32;
        node_str[1] = (is_large)? str[it_hyphen+1] : str[it_hyphen+1] - 32;
        node_b = str_to_node(node_str);
        _big_caves[node_b] = is_large;
    } else if (char_left == 3) {
        node_b = END;
    } else if (char_left == 5) {
        node_b = START;
    } else {
        return false;
    }

    // we can never go back to START
    if (node_b != START && !add_connection(node_a, node_b)) return false;
    if (node_a != START && !add_connection(node_b, node_a)) return false;

    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Map map;
    map.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!map.add_node(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    map.calc_paths();

    const uint16_t answer1 = map.simple_visit_count;
    const uint32_t answer2 = map.part_two_count;

    file.close();
    map.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 12 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %i\n", answer2);

    return 0;
}
