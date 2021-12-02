#ifndef STRTOINT_H
#define STRTOINT_H

inline int ascii_isdigit(const int c) {
    return (c >= '0' && c <= '9' ? 1 : 0);
}

inline int ascii_isalpha(const int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ? 1 : 0);
}

// barebone string to int
// no limit check, only base 10
// not safe
int strtoint(const char* str) {
    const char* i = str;
    int c = *i++;
    int negative = 0;
    int ret = 0;

    if (c == '-') {
        negative = 1;
        c = *i++;
    } else if (c == '+') {
        c = *i++;
    }

    while (c != 0) {
        if (ascii_isdigit(c)) c -= '0';
        else break;

        if (c >= 10) break;

        ret *= 10; // this can overflow
        ret += c;
        c = *i++;
    }

    if (negative) ret = -ret;
    return ret;
}

# endif // STRTOINT_H
