#include <stdio.h>
#include <string.h>

#include "file.h"
#include "strtoint.h"
#include "timer.h"

const uint16_t INPUT_MAX = 64;
const uint8_t DICE_MAX = 100;
const uint16_t WINNING_SCORE = 1000;
const uint8_t N_ROLLS = 3;
const uint8_t N_PLAYERS = 2;
const uint8_t BOARD_LIMIT = 10;

const uint8_t DIRAC_WINNING = 21;
const uint8_t DIRAC_ROLL = 7;
const uint8_t dirac_dice[] {1, 3, 6, 7, 6, 3, 1};

typedef struct Game {
    uint8_t pos[N_PLAYERS];
    uint16_t score[N_PLAYERS];
    uint64_t total_rolls;
    uint8_t next_roll;

    uint8_t roll() {
        if (next_roll > DICE_MAX) next_roll = 1;
        next_roll += 1;
        return next_roll;
    }
} Game;

template<class T> 
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

typedef struct Board {
    void init();
    void destroy();
    bool read_state(const char* str);

    uint32_t answer1() const { return _answer1; }
    uint64_t answer2() const { return _answer2; }
    void play();
    void play_dirac();

private:
    uint8_t dirac_roll();

    uint32_t _answer1;
    uint64_t _answer2;

    uint8_t _starting_pos[N_PLAYERS];
    Game _regular_game;
} Board;

void Board::init() {
    memset(&_regular_game, 0, sizeof(_regular_game));
    memset(_starting_pos, 0, sizeof(_starting_pos));
    _answer1 = 0;
    _answer2 = 0;
}

void Board::destroy() {
    // nothing
}

// BETTER, I feel there's a way to avoid so much copying
// or using cache better while iterating this giant multi array
void Board::play_dirac() {
    // Count of actual game(s) at a specific game state for all combinaisons of:
    // P1 score, P1 position, P2 score, P2 position.
    typedef uint64_t GameState[DIRAC_WINNING+1][BOARD_LIMIT][DIRAC_WINNING+1][BOARD_LIMIT];
    typedef GameState Multiverse[N_PLAYERS];

    Multiverse multiverse;
    memset(multiverse, 0, sizeof(multiverse));

    // set the starting state
    // indexing board position to 0
    multiverse[0][0][_starting_pos[0]-1][0][_starting_pos[1]-1] = 1;

    // we're only done when ALL universes are in a state where
    // both players are at winning score
    bool all_winners = false;
    while (all_winners == false) {
        all_winners = true;

        // reset this part of the multiverse
        memset(&multiverse[1], 0, sizeof multiverse[1] - sizeof multiverse[1][DIRAC_WINNING]);
        memcpy(&multiverse[1][DIRAC_WINNING], &multiverse[0][DIRAC_WINNING], sizeof multiverse[0][DIRAC_WINNING]);

        // copy the player one multiverses into player two's
        for (uint8_t s1 = 0; s1 < DIRAC_WINNING; ++s1) {
            for (uint8_t p1 = 0; p1 < BOARD_LIMIT; ++p1) {
                memcpy(&multiverse[1][s1][p1][DIRAC_WINNING], &multiverse[0][s1][p1][DIRAC_WINNING],
                    sizeof(multiverse[1][s1][p1][DIRAC_WINNING]));
            }
        }
        // roll for all our game states for player 1
        for (uint8_t s1 = 0; s1 < DIRAC_WINNING; ++s1) {
        for (uint8_t p1 = 0; p1 < BOARD_LIMIT;   ++p1) {
        for (uint8_t s2 = 0; s2 < DIRAC_WINNING; ++s2) {
        for (uint8_t p2 = 0; p2 < BOARD_LIMIT;   ++p2) {
            const uint64_t& n_universe = multiverse[0][s1][p1][s2][p2];
            // Don't roll if no universe are in that state
            if (n_universe == 0) continue;
            all_winners = false;
            // generate all possible rolls and update universe counts
            for (uint8_t roll = 0; roll < DIRAC_ROLL; ++roll) {
                const uint8_t new_position = (p1 + roll + 3) % 10;
                const uint8_t new_score = min(DIRAC_WINNING, (uint8_t)(s1 + new_position + 1));
                multiverse[1][new_score][new_position][s2][p2] += dirac_dice[roll] * n_universe;
            }
        }}}}

        // Now the same for the other player
        memset(&multiverse[0], 0, sizeof(multiverse[0]) - sizeof(multiverse[0][DIRAC_WINNING]));
        memcpy(&multiverse[0][DIRAC_WINNING], &multiverse[1][DIRAC_WINNING], sizeof(multiverse[1][DIRAC_WINNING]));
        for (uint8_t s1 = 0; s1 < DIRAC_WINNING; ++s1) {
            for (uint8_t p1 = 0; p1 < BOARD_LIMIT; ++p1) {
                memcpy(&multiverse[0][s1][p1][DIRAC_WINNING], &multiverse[1][s1][p1][DIRAC_WINNING],
                    sizeof(multiverse[0][s1][p1][DIRAC_WINNING]));
            }
        }
        // roll for all our game states
        for (uint8_t s1 = 0; s1 < DIRAC_WINNING; ++s1) {
        for (uint8_t p1 = 0; p1 < BOARD_LIMIT;   ++p1) {
        for (uint8_t s2 = 0; s2 < DIRAC_WINNING; ++s2) {
        for (uint8_t p2 = 0; p2 < BOARD_LIMIT;   ++p2) {
            const uint64_t& n_universe = multiverse[1][s1][p1][s2][p2];
            // Don't roll if no universe are in that state
            if (n_universe == 0) continue;
            all_winners = false;
            // generate all possible rolls and update universe counts
            for (uint8_t roll = 0; roll < DIRAC_ROLL; ++roll) {
                const uint8_t new_position = (p2 + roll + 3) % 10;
                const uint8_t new_score = min(DIRAC_WINNING, (uint8_t)(s2 + new_position + 1));
                multiverse[0][s1][p1][new_score][new_position] += dirac_dice[roll] * n_universe;
            }
        }}}}
    }

    // count all instance of p1 with a winning score
    // -> p2 is NOT at DIRAC_WINNING
    uint64_t p1_winning_count = 0;
    for (uint8_t p1 = 0; p1 < BOARD_LIMIT;   ++p1) {
    for (uint8_t s2 = 0; s2 < DIRAC_WINNING; ++s2) {
    for (uint8_t p2 = 0; p2 < BOARD_LIMIT;   ++p2) {
        p1_winning_count += multiverse[0][DIRAC_WINNING][p1][s2][p2];
    }}}

    // count all instance of p2 with a winning score
    // -> p1 is NOT at DIRAC_WINNING
    uint64_t p2_winning_count = 0;
    for (uint8_t s1 = 0; s1 < DIRAC_WINNING; ++s1) {
    for (uint8_t p1 = 0; p1 < BOARD_LIMIT;   ++p1) {
    for (uint8_t p2 = 0; p2 < BOARD_LIMIT;   ++p2) {
        p2_winning_count += multiverse[0][s1][p1][DIRAC_WINNING][p2];
    }}}

  _answer2 = (p1_winning_count > p2_winning_count)? p1_winning_count : p2_winning_count;
}

void Board::play() {
    for (uint8_t i = 0; i < N_PLAYERS; ++i) {
        _regular_game.pos[i] = _starting_pos[i];
    }

    while (true) {
        for (uint8_t i = 0; i < N_PLAYERS; ++i) {
            const uint16_t advance = _regular_game.roll() + _regular_game.roll() + _regular_game.roll();
            _regular_game.total_rolls += 3;

            const uint8_t remain = (_regular_game.pos[i] + advance) % BOARD_LIMIT;
            const uint8_t new_pos = (remain == 0)? BOARD_LIMIT : remain;
            _regular_game.pos[i] = new_pos;
            _regular_game.score[i] += new_pos;

            if (_regular_game.score[i] < WINNING_SCORE) continue;

            _answer1 = (i == 1)? _regular_game.total_rolls * _regular_game.score[0] : _regular_game.total_rolls * _regular_game.score[1];
            return;
        }
    }
}

bool Board::read_state(const char* str) {
    uint8_t it = 0;
    while (str[it] != 0) {
        if (str[it] == ':') break;
        it += 1;
    }
    if (str[it] != ':') return false;
    it += 2; // FIXME, unsafe

    uint8_t value = 0;
    while (str[it] != '\0') {
        if (ascii_isdigit(str[it])) {
            value *= 10;
            value += str[it] - '0';
        }
        ++it;
    }

    for (uint8_t i = 0; i < N_PLAYERS; ++i) {
        if (_starting_pos[i] == 0) {
            _starting_pos[i] = value;
            break;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Board board;
    board.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX) > 1) {
        if (!board.read_state(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    board.play();
    board.play_dirac();

    const uint32_t answer1 = board.answer1();
    const uint64_t answer2 = board.answer2();

    file.close();
    board.destroy();

    const uint64_t completion_time = timer_stop();
    printf("Day 21 completion time: %" PRIu64 "µs\n", completion_time);

    printf("Answer 1 = %u\n", answer1);
    printf("Answer 2 = %lu\n", answer2);

    return 0;
}
