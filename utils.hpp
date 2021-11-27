#ifndef INCLUDE_HTTP_UTILS
#define INCLUDE_HTTP_UTILS

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>


size_t get_digits(size_t n) {
    size_t digits = 1;
    for (; n /= 10; digits++);
    return digits;
}

struct string_view;

struct string_pos {
    size_t start;
    size_t size;

    constexpr string_pos() : start(0), size(0) {}
    constexpr string_pos(size_t start, size_t size) : start(start), size(size) {}

    inline string_pos operator+(size_t n) const {
        return string_pos(start + n, size - n);
    }
    inline string_pos &operator+=(size_t n) {
        *this = *this + n;
        return *this;
    }

    string_view get_str_view(const char *buffer) const;
};

struct string_view {
    const char *str;
    size_t size;

    string_view() = default;
    constexpr string_view(const char *str) : str(str), size(strlen(str)) {}
    constexpr string_view(const char *str, size_t size) : str(str), size(size) {}
    constexpr string_view(string_pos view, const char *str) : str(str + view.start), size(view.size) {}

    inline const char *operator[](int index) const {
        return str + index;
    }
    inline bool operator==(const string_view &other) const {
        if (other.size != size) return false;
        return memcmp(str, other.str, size);
    }
    inline string_view operator+(size_t n) {
        assert(size > n);
        return string_view(str + n, size - n);
    }
    inline string_view &operator+=(size_t n) {
        *this = *this + n;
        return *this;
    }

    inline void print(FILE *f = stdout) const {
        fwrite(str, sizeof(char), size, f);
    }

    inline char *cpy(char *dest) const {
        return size + (char *)memcpy(dest, str, size); 
    }
    
    char *make_c_str() {
        char *c_str = (char *)malloc(size + 1);
        c_str[size] = '\0';
        memcpy(c_str, str, size);

        return c_str;
    }

    bool get_num(size_t &num) {
        int lower = 0;
        for (; lower < size && lower > -1; lower++) if (isdigit(str[lower])) break;
        if (lower == size) return false;

        int max = lower;
        for (; isdigit(str[max]); max++);
        max--;

        num = 0;
        size_t base = 1;
        for (int i = max; i >= lower; i--, base *= 10) {
            num += base * (str[i] - '0');
        }

        return true;
    }
};

string_view string_pos::get_str_view(const char *buffer) const {
    return string_view(*this, buffer);
}

struct cmp_str
{
    bool operator()(const string_view &a, const string_view &b) const
    {
        if (a.size < b.size) return true;
        if (a.size > b.size) return false;
        
        return memcmp(a.str, b.str, a.size) < 0;
    }
};

struct cmp_str_case_insensitive
{
    bool operator()(const string_view &a, const string_view &b) const
    {
        if (a.size < b.size) return true;
        if (a.size > b.size) return false;

        for (size_t i = 0; i < a.size; i++) {
            if (tolower(a[i][0]) == tolower(b[i][0])) continue;
            return (tolower(a[i][0]) - tolower(b[i][0])) < 0;
        }

        return false;
    }
};

bool str_find(const string_view &str, const string_view &delimiter, size_t &pos) {
    if (str.size < delimiter.size) return false;
    for (size_t i = 0; i <= str.size - delimiter.size; i++) {
        if (!memcmp(str[i], delimiter[0], delimiter.size)) {
            pos = i;
            return true;
        }
    }

    return false;
}

template<typename T>
struct queue {
    T *arr;
    size_t capacity;
    size_t used;
    size_t front_i;

    void init(size_t begin_size = 512) {
        arr = realloc((T *)malloc(begin_size * sizeof(T)));
        capacity = begin_size;
        front_i = 0;
        used = 0;
    }

    void resize(){
        size_t newcapacity = capacity << 1;
        T *newarr = realloc((T *)malloc(newcapacity * sizeof(T)));

        for (size_t i = front_i, j = 0; j < capacity; i = (i + 1) % capacity, j++) {
            newarr[j] = arr[i];
        }

        free(dealloc(arr));
        arr = newarr;
        capacity = newcapacity;
        front_i = 0;
    }

    void push(T obj) {
        if (used == capacity) resize();
        size_t index = (front_i + used) % capacity;

        printf("%lu %lu\n", index, capacity);
        arr[index] = obj;
        used++;
    }

    void pop() {
        if (used == 0) return;
        front_i = (front_i + 1) % capacity;
        used--;
    }

    inline T &front() {
        return arr[front_i];
    }

    inline size_t size() {
        return used;
    }

    inline bool empty(){
        return used == 0;
    }

    inline void erase(){
        free(dealloc(arr));
    }
};

#endif