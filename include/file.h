#ifndef FILE_H
#define FILE_H

#include <stdio.h>

// useless wrapper around fopen for now
// I might access the file directly in the
// future and avoid the byte stream
// for faster I/O
FILE* open_file(const char* f) {
    return fopen(f, "r");
}

int close_file(FILE* f) {
    return fclose(f);
}

#endif // FILE_H
