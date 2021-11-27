#ifndef INCLUDE_HTTP_TYPES
#define INCLUDE_HTTP_TYPES

#include "../utils.hpp"

#include <map>

enum request_t {
    rother = 0, rget, rpost, rput, rhead, rdelete, rtrace, roptions, rconnect, rpatch
};

enum httpversion {
    httpother = 0, http10, http11, http2
};

enum standard_header {
    hother = 0,
    haccept, 
    hacceptenc, 
    hacceptcharset,
    hcookie, 
    hsetcookie, 
    hcontenttype, 
    hcontentlength, 
    hcontentenc,
    hdate, 
    hserver,
    hhost
};

namespace HttpChars {
    constexpr string_view colon_space(": ", 2);
    constexpr string_view colon(":", 1);
    constexpr string_view crlf("\r\n", 2);
    constexpr string_view space(" ", 1);
}

std::map<const string_view, const standard_header, cmp_str_case_insensitive> str_to_header_t = {
    {string_view("Accept"), haccept},
    {string_view("Accept-Encoding"), hacceptenc},
    {string_view("Accept-Charset"), hacceptcharset},
    {string_view("Cookie"), hcookie},
    {string_view("Set-Cookie"), hsetcookie},
    {string_view("Content-Type"), hcontenttype},
    {string_view("Content-Length"), hcontentlength},
    {string_view("Content-Encoding"), hcontentenc},
    {string_view("Date"), hdate},
    {string_view("Server"), hserver},
    {string_view("Host"), hhost}
};

std::map<const standard_header, const string_view> header_to_str = {
    {haccept, string_view("Accept")},
    {hacceptenc, string_view("Accept-Encoding")},
    {hacceptcharset, string_view("Accept-Charset")},
    {hcookie, string_view("Cookie")},
    {hsetcookie, string_view("Set-Cookie")},
    {hcontenttype, string_view("Content-Type")},
    {hcontentlength, string_view("Content-Length")},
    {hcontentenc, string_view("Content-Encoding")},
    {hdate, string_view("Date")},
    {hserver, string_view("Server")},
    {hhost, string_view("Host")}
};

std::map<const string_view, const request_t, cmp_str> str_to_request_t = {
    {string_view("GET"), rget},
    {string_view("POST"), rpost},
    {string_view("PUT"), rput},
    {string_view("HEAD"), rhead},
    {string_view("DELETE"), rdelete},
    {string_view("TRACE"), rtrace},
    {string_view("OPTIONS"), roptions},
    {string_view("CONNECT"), rconnect},
    {string_view("PATCH"), rpatch}
};

std::map<const string_view, const httpversion, cmp_str> str_to_httpversion = {
    {string_view("HTTP/1.0"), http10},
    {string_view("HTTP/1.1"), http11},
    {string_view("HTTP/2"), http2}
};

std::map<const httpversion, const string_view> httpversion_to_str = {
    {http10, string_view("HTTP/1.0")},
    {http11, string_view("HTTP/1.1")},
    {http2, string_view("HTTP/2")}
};

template<typename Key, typename T, typename cmp>
T get_from_map(std::map<Key, T, cmp> &map, Key &k, T &notfound) {
    if (map.count(k)) return map[k];

    return notfound;
}

#endif