#ifndef INCLUDE_UTILS
#define INCLUDE_UTILS

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>


size_t get_digits(size_t n);

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

    inline string_view() = default;
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
    
    char *make_c_str();
    bool get_num(size_t &num);
};

struct cmp_str {
    bool operator()(const string_view &a, const string_view &b) const;
};

struct cmp_str_case_insensitive {
    bool operator()(const string_view &a, const string_view &b) const;
};

bool str_find(const string_view &str, const string_view &delimiter, size_t &pos);

#endif