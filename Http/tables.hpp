#ifndef INCLUDE_HTTP_TYPES
#define INCLUDE_HTTP_TYPES

#include "utils.hpp"

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
    {string_view("Accept", 6), haccept},
    {string_view("Accept-Encoding", 15), hacceptenc},
    {string_view("Accept-Charset", 14), hacceptcharset},
    {string_view("Cookie", 6), hcookie},
    {string_view("Set-Cookie", 10), hsetcookie},
    {string_view("Content-Type", 12), hcontenttype},
    {string_view("Content-Length", 14), hcontentlength},
    {string_view("Content-Encoding", 16), hcontentenc},
    {string_view("Date", 4), hdate},
    {string_view("Server", 6), hserver},
    {string_view("Host", 4), hhost}
};

std::map<const standard_header, const string_view> header_to_str = {
    {haccept, string_view("Accept", 6)},
    {hacceptenc, string_view("Accept-Encoding", 15)},
    {hacceptcharset, string_view("Accept-Charset", 14)},
    {hcookie, string_view("Cookie", 6)},
    {hsetcookie, string_view("Set-Cookie", 10)},
    {hcontenttype, string_view("Content-Type", 12)},
    {hcontentlength, string_view("Content-Length", 14)},
    {hcontentenc, string_view("Content-Encoding", 16)},
    {hdate, string_view("Date", 4)},
    {hserver, string_view("Server", 6)},
    {hhost, string_view("Host", 4)}
};

std::map<const string_view, const request_t, cmp_str> str_to_request_t = {
    {string_view("GET", 3), rget},
    {string_view("POST", 4), rpost},
    {string_view("PUT", 3), rput},
    {string_view("HEAD", 4), rhead},
    {string_view("DELETE", 6), rdelete},
    {string_view("TRACE", 5), rtrace},
    {string_view("OPTIONS", 7), roptions},
    {string_view("CONNECT", 7), rconnect},
    {string_view("PATCH", 5), rpatch}
};

std::map<const string_view, const httpversion, cmp_str> str_to_httpversion = {
    {string_view("HTTP/1.0", 8), http10},
    {string_view("HTTP/1.1", 8), http11},
    {string_view("HTTP/2", 6), http2}
};

std::map<const httpversion, const string_view> httpversion_to_str = {
    {http10, string_view("HTTP/1.0", 8)},
    {http11, string_view("HTTP/1.1", 8)},
    {http2, string_view("HTTP/2", 6)}
};

template<typename Key, typename T, typename cmp>
T get_from_map(std::map<Key, T, cmp> &map, Key &k, T &notfound) {
    if (map.count(k)) return map[k];

    return notfound;
}

#endif