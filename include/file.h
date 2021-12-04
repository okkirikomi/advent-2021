#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// TODO use _fstat and _open on Windows
#ifdef _WIN32
#error File.h not implemented for windows.
#endif

static const int LINE_FEED = 10;

typedef struct File {
    bool open(const char* path);
    void close();
    int readline(char* out, const int buffer_size);

private:
    char* _data;
    off_t _size;
    off_t _it;
} File;

void File::close() {
    free(_data);
}

int File::readline(char* out, const int buffer_size) {
    if (out == NULL) return 0;
    if (_it >= _size) return 0;

    const off_t start = _it;
    // we can be smarter by reading words or something
    // but this is good enough for now
    while (_it < _size && _data[_it] != LINE_FEED) {
        if (_it - start >= buffer_size - 1) {
            _it = start;
            return 0;
        }
        out[_it-start] = _data[_it];
        ++_it;
    }
    const int n_read = _it - start + 1;
    out[_it-start] = '\0';

    // go over the line ending
    ++_it;
    return n_read;
}

bool File::open(const char* path) {
    bool ok = false;

    int descriptor = ::open(path, O_RDONLY);
    if (descriptor == -1) return false;

    struct stat buffer;
    int status = fstat(descriptor, &buffer);
    if (status != 0) goto beach;

    // TODO, check if using mmap directly is faster for big enough files
    _data = (char*) malloc(buffer.st_size * sizeof(char));
    if (_data == NULL) goto beach;

    if(read(descriptor, _data, buffer.st_size)!= buffer.st_size) {
        free(_data);
        goto beach;
    }

    _size = buffer.st_size;
    _it = 0;
    ok = true;
beach:
    ::close(descriptor);
    return ok; 
}

#endif // FILE_H
