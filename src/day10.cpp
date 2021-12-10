#include <stdio.h>

#include "file.h"
#include "radix_sort64.h"
#include "stack.h"
#include "timer.h"

static const uint8_t INPUT_MAX = 128;
static const uint8_t STACK_MAX = 64;
static const uint8_t MAX_INCOMPLETE = 64;

typedef struct Parser {
    void init();
    void close();
    bool parse_line(const char* str);

    uint32_t error_score() const { return _error_score; }
    uint64_t completion_score();

private:
    uint32_t _error_score;
    uint64_t _completion_scores[MAX_INCOMPLETE];
    uint8_t _n_incomplete;
    Stack<char> _stack;

} Parser;

uint64_t Parser::completion_score() {
    if (_n_incomplete == 0) return 0;
    // BETTER, is there a faster way to find middle score than sorting?
    radix_sort64(_completion_scores, _n_incomplete);
    return _completion_scores[_n_incomplete/2];
}

bool Parser::parse_line(const char* str) {
    size_t it = 0;
    char last;
    _stack.clear();
    while (true) {
        switch (str[it]) {
            case '(': if(!_stack.push('(')) return false; break;
            case '[': if(!_stack.push('[')) return false; break;
            case '{': if(!_stack.push('{')) return false; break;
            case '<': if(!_stack.push('<')) return false; break;
            case ')':
                if (!_stack.pop(last)) return false;
                if (last != '(') {
                    _error_score += 3;
                    return true;
                }
                break;
            case ']':
                if (!_stack.pop(last)) return false;
                if (last != '[') {
                    _error_score += 57;
                    return true;
                }
                break;
            case '}':
                if (!_stack.pop(last)) return false;
                if (last != '{') {
                    _error_score += 1197;
                    return true;
                }
                break;
            case '>':
                if (!_stack.pop(last)) return false;
                if (last != '<') {
                    _error_score += 25137;
                    return true;
                }
                break;

            case '\0': goto incomplete;
            default: return false;
        }
        ++it;
    }
incomplete:
    uint64_t completion_score = 0;
    while(_stack.pop(last)) {
        completion_score *= 5;
        switch (last) {
            case '(': completion_score += 1; break;
            case '[': completion_score += 2; break;
            case '{': completion_score += 3; break;
            case '<': completion_score += 4; break;
            default: return false;
        }
    }
    _completion_scores[_n_incomplete] = completion_score;
    if ((++_n_incomplete) == MAX_INCOMPLETE) return false;
    return true;
}

void Parser::close() {
    _stack.destroy();
}

void Parser::init() {
    _error_score = 0;
    _n_incomplete = 0;
    _stack.init(STACK_MAX);
}

int main(int argc, char **argv)
{
    timer_start();

    if (argc < 1) {
        printf("No input!\n");
        return -1;
    }

    Parser parser;
    parser.init();

    File file;
    if(file.open(argv[1]) == false) {
        printf("Couldn't read file %s\n", argv[1]);
        return -1;
    }

    char str[INPUT_MAX];
    while (file.readline(str, INPUT_MAX)) {
        if (!parser.parse_line(str)) {
            printf("Error with input.\n");
            return -1;
        }
    }

    const uint32_t answer1 = parser.error_score();
    const uint64_t answer2 = parser.completion_score();

    file.close();

    const uint64_t completion_time = timer_stop();
    printf("Day 10 completion time: %" PRIu64 "µs\n", completion_time);
    printf("Answer 1 = %i\n", answer1);
    printf("Answer 2 = %" PRIu64 "\n", answer2);

    return 0;
}
