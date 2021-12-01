#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

template <class T>
struct Ringbuffer
{
    inline size_t size() const { return _size; }
    T& operator[](size_t i) const { return _data[i]; }
    T* data() { return _data; }
    void init(const size_t);
    void clear();
    void free();
    T const sum();
    T& last();
    void push(T t);
private:
    size_t _size;
    size_t _idx;
    T* _data;
};

template <class T>
T& Ringbuffer<T>::last() {
    if (_idx == 0) return _data[_size-1];
    return _data[_idx-1];
}

// TODO implement move if using not built-in type
template <class T>
void Ringbuffer<T>::push(T t) {
    _data[_idx] = t;
    if ((++_idx) >= _size) _idx = 0;
}

template <class T>
void Ringbuffer<T>::clear() {
    for (size_t i = 0; i < _size; ++i) {
        _data[i] = 0;
    }
    _idx = 0;
}

template <class T>
void Ringbuffer<T>::init(const size_t size) {
    assert(size > 0);
    _size = size;
    _idx = 0;
    _data = (T*) malloc(sizeof(T) * _size);
}

template <class T>
const T Ringbuffer<T>::sum() {
    T total = 0;
    for (size_t i = 0; i < _size; ++i) {
        total += _data[i];
    }
    return total;
}

template <class T>
void Ringbuffer<T>::free() {
    ::free(_data);
}

#endif // RING_BUFFER_H
