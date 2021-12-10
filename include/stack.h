#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <utility>

// Simple stack with fixed memory
// Implement custom allocator if needed

template <class T>
struct Stack
{
    bool init(const int32_t size);
    void destroy();
    bool push(T&&);
    bool push(const T&);
    bool pop(T&);
    bool pop(T&&);
    inline void clear() { _pos = -1; };
    inline int32_t size() const { return _pos+1; }
private:
    int32_t _size;
    int32_t _pos;
    T* _data;
};

template <class T>
bool Stack<T>::init(const int32_t size) {
    assert(size > 0);
    _size = size;
    _pos = -1;
    _data = (T*) malloc(sizeof(T) * size);
    return (_data != NULL);
}

template <class T>
void Stack<T>::destroy() {
    free(_data); 
}

template <class T>
bool Stack<T>::push(const T& val) {
    if (_pos == _size) return false;

    _data[++_pos] = val;
    return true;
}

template <class T>
bool Stack<T>::push(T&& val) {
    if (_pos == _size) return false;

    _data[++_pos] = std::move(val);
    return true;
}

template <class T> 
bool Stack<T>::pop(T&& ret) {
    if (_pos == -1) return false;

    std::swap(ret, _data[_pos]);
    _pos -= 1;
    return true;
}

template <class T> 
bool Stack<T>::pop(T& ret) {
    if (_pos == -1) return false;

    ret = _data[_pos--] ;
    return true;
}

#endif // STACK_H
