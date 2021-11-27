#if 0
#ifndef INCLUDE_MY_ALLOC
#define INCLUDE_MY_ALLOC

#include <stdlib.h>
#include <map>

struct Allocation {
    const char *file;
    int line;
    const char *function;

    void *ptr;
};

std::map<void *, Allocation> allocs;

template<typename T>
inline T *reg_alloc_func(T *ptr, const char *file, int line, const char *function) {
    Allocation a;
    a.file = file;
    a.line = line;
    a.function = function;
    a.ptr = ptr;

    allocs[ptr] = a;

    return ptr;
}

template<typename T>
inline T *dereg_alloc_func(T *ptr) {
    allocs.erase(ptr);
    return ptr;
}

#define realloc(ptr) reg_alloc_func(ptr, __FILE__, __LINE__, __func__)
#define dealloc(ptr) dereg_alloc_func(ptr)

#endif
#endif