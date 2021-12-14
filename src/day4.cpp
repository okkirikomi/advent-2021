#include <stdio.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

static const size_t MAX_BOARDS = 128;
static const size_t BOARD_SIDE = 5;
static const size_t BOARD_SIZE = BOARD_SIDE * BOARD_SIDE;
static const size_t MAX_DRAWS = 128;
static const size_t INPUT_MAX = 512;

static const unsigned char DRAWN_VALUE = 0xFF;

// last element hack, used to flag bingo
// to avoid resizing our array of boards
typedef unsigned char Board[BOARD_SIZE+1];

typedef struct Winner {
    size_t last_called;
    size_t id;
} Winner;

typedef struct Boards {
    void init();
    void destroy();
    uint unmarked_sum(const size_t board_id);
    int mark(const unsigned char value, Winner& winner, const bool checkbingo);
    void add_row(const char* str);
    bool bingo_all_boards(unsigned char* draws, const size_t draw_size, uint* first_score, uint* last_score);

private:
    bool check_bingo(const Board& board, size_t last_cell);
    void increase_cell();
    size_t _current_cell;
    size_t _n_boards;
    Board* _boards;

} Boards;

void Boards::increase_cell() {
    ++_current_cell;
    if (_current_cell >= BOARD_SIZE) {
        _boards[_n_boards][BOARD_SIZE] = 0; // last element is bingo flag
        if (_n_boards < MAX_BOARDS) _n_boards += 1;
        _current_cell = 0;
    }
}

void Boards::add_row(const char* str) {
    size_t it = 0;
    unsigned char value = 0;
    char last_char = 0;
    while (str[it] != '\0') {
        if (str[it] == ' ') {
            last_char = ' ';
            ++it;
            continue;
        }
        if (last_char == ' ' && it > 1) {
            _boards[_n_boards][_current_cell] = value;
            increase_cell();
            value = 0;
        }
        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            value *= 10;
            value += c;
        }
        last_char = str[it];
        ++it;
    }
    _boards[_n_boards][_current_cell] = value;
    increase_cell();
}

void Boards::destroy() {
    free(_boards);
}

void Boards::init() {
    _boards = (Board*) malloc(sizeof(Board) * BOARD_SIZE * MAX_BOARDS);
    if (_boards == NULL) abort();
    _current_cell = 0;
    _n_boards = 0;
}

// only check the two rows the last cell is in
bool Boards::check_bingo(const Board& board, size_t last_cell) {
    // horizontal row
    const size_t x_mod = last_cell % (BOARD_SIDE);
    const size_t x_end = last_cell + (5 - x_mod);
    const size_t x_start = last_cell - x_mod;
    bool bingo = true;
    for (size_t i = x_start; i < x_end; ++i) {
        if (board[i] != DRAWN_VALUE) {
            bingo = false;
            break;
        }
    }
    if (bingo) return true;

    // vertical row
    const size_t n_row = last_cell / (BOARD_SIDE);
    const size_t y_end = last_cell + ((5 - n_row)* BOARD_SIDE);
    const size_t y_start = last_cell - (n_row * BOARD_SIDE);
    bingo = true;
    for (size_t i = y_start; i < y_end; i += 5) {
        if (board[i] != DRAWN_VALUE) {
            bingo = false;
            break;
        }
    }
    return bingo;
}

// if the board has not won yet,
// set a matching tile to the drawn value if present
// if we got a match, check if it completes a bingo
// returns the number of bingos by this value
int Boards::mark(const unsigned char value, Winner& winner, const bool check) {
    int bingo = 0;
    for (size_t i = 0; i < _n_boards; ++i) {
        if (_boards[i][BOARD_SIZE] != 0) continue;
        for (size_t cell = 0; cell < BOARD_SIZE; ++cell) {
            if (_boards[i][cell] != value) continue;
            _boards[i][cell] = DRAWN_VALUE;
            if (check && check_bingo(_boards[i], cell)) {
                _boards[i][BOARD_SIZE] = DRAWN_VALUE;
                winner.last_called = value;
                winner.id = i;
                ++bingo;
                // FIXME, don't return just the last bingo if needed
            }
            // a value can only appear once in a board
            break;
        }
    }
    return bingo;
}

uint Boards::unmarked_sum(const size_t board_id) {
    uint sum = 0;
    const Board& board = _boards[board_id];
    for (size_t i = 0; i < BOARD_SIZE; ++i) {
        if (board[i] == DRAWN_VALUE) continue;
        sum += (uint) board[i];
    }
    return sum;
}

bool Boards::bingo_all_boards(unsigned char* draws, const size_t draw_size, uint* first_score, uint* last_score) {
    Winner first_winner = {};
    Winner last_winner = {};

    int bingo = false;
    size_t n_bingo = 0;

    for (size_t i = 0; i < draw_size && n_bingo < _n_boards; ++i) {
        Winner winner;
        const bool checkbingo = (i > 4)? true : false;
        bingo = mark(draws[i], winner, checkbingo);
        if (bingo == 0) continue;

        n_bingo += bingo;
        // keep the first and last winners
        // FIXME, this won't work if first winner not alone on its mark
        if (n_bingo == 1) first_winner = winner;
        if (n_bingo >= _n_boards) last_winner = winner;
    }
    // we didn't end on a bingo, bail out
    if (!bingo) return false;

    *first_score = unmarked_sum(first_winner.id) * (uint) first_winner.last_called;
    *last_score = unmarked_sum(last_winner.id) * (uint) last_winner.last_called;

    return true;
}

static size_t parse_draws(const char* str, unsigned char* draws, const size_t draw_size) {
    size_t it = 0;
    size_t draw_i = 0;
    unsigned char draw = 0;
    while (str[it] != '\0') {
        if (str[it] == ',') {
            draws[draw_i] = draw;
            if (draw_i >= draw_size) return 0;
            ++draw_i;
            ++it;
            draw = 0;
        }

        if (ascii_isdigit(str[it])) {
            const char c = str[it] - '0';
            draw *= 10;
            draw += c;
        }
        if (str[it] != '\0') ++it;
    }
    draws[draw_i] = draw;
    return draw_i + 1;
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

    Boards boards;
    boards.init();

    unsigned char draws[MAX_DRAWS];
    size_t draw_size = 0;
    char str[INPUT_MAX];
    int n_line = 0;
    while (true) {
        const int n_read = file.readline(str, INPUT_MAX);
        if (n_read == 0) break;

        if (n_line > 1 && n_read > 1) {
            boards.add_row(str);
        } else if (n_line == 0) {
            draw_size = parse_draws(str, draws, MAX_DRAWS);
            if (draw_size == 0) {
                printf("Error parsing draws.\n");
                return -1;
            }
        }
        ++n_line;
    }

    uint answer1;
    uint answer2;
    if(!boards.bingo_all_boards(draws, draw_size, &answer1, &answer2))
        return -1;

    file.close();
    boards.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 4 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %u\n", answer2);

    return 0;
}
