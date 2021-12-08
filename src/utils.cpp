#include "utils.hpp"

size_t get_digits(size_t n) {
    size_t digits = 1;
    for (; n /= 10; digits++);
    return digits;
}

    
char *string_view::make_c_str() {
    char *c_str = (char *)malloc(size + 1);
    c_str[size] = '\0';
    memcpy(c_str, str, size);

    return c_str;
}

bool string_view::get_num(size_t &num) {
    size_t lower = 0;
    for (; lower < size; lower++) if (isdigit(str[lower])) break;
    if (lower == size) return false;

    int max = lower;
    for (; isdigit(str[max]); max++);
    max--;

    num = 0;
    size_t base = 1;
    for (size_t i = max; i >= lower; i--, base *= 10) {
        num += base * (str[i] - '0');
    }

    return true;
}

string_view string_pos::get_str_view(const char *buffer) const {
    return string_view(*this, buffer);
}

bool cmp_str::operator()(const string_view &a, const string_view &b) const
{
    if (a.size < b.size) return true;
    if (a.size > b.size) return false;
    
    return memcmp(a.str, b.str, a.size) < 0;
}

bool cmp_str_case_insensitive::operator()(const string_view &a, const string_view &b) const
{
    if (a.size < b.size) return true;
    if (a.size > b.size) return false;

    for (size_t i = 0; i < a.size; i++) {
        if (tolower(a[i][0]) == tolower(b[i][0])) continue;
        return (tolower(a[i][0]) - tolower(b[i][0])) < 0;
    }

    return false;
}

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