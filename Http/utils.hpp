#ifndef INCLUDE_HTTP_UTILS
#define INCLUDE_HTTP_UTILS

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>


size_t get_digits(size_t n) {
    size_t digits = 1;
    for (; n /= 10; digits++);
    return digits;
}

struct string_pos {
    size_t start;
    size_t size;

    constexpr string_pos() : start(0), size(0) {}
    constexpr string_pos(size_t start, size_t size) : start(start), size(size) {}

    inline string_pos operator+(size_t n) {
        return string_pos(start + n, size - n);
    }
    inline string_pos &operator+=(size_t n) {
        *this = *this + n;
        return *this;
    }
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

    bool get_num(size_t num) {
        int lower = 0;
        for (; lower < size && lower > -1; lower++) if (isdigit(str[lower])) break;
        if (lower == size) return false;

        int max = size - 1;
        for (;; max--) if (isdigit(str[lower])) break;

        num = 0;
        size_t base = 1;
        for (int i = max; i >= lower; i--, base *= 10) {
            num += base * (str[i] - '0');
        }

        return true;
    }
};

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

        return true;
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

#endif