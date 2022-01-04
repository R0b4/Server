#ifndef INCLUDE_HTTP_TYPES
#define INCLUDE_HTTP_TYPES

#include <server/utils.hpp>

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

extern std::map<const string_view, const standard_header, cmp_str_case_insensitive> str_to_header_t;
extern std::map<const standard_header, const string_view> header_to_str;

extern std::map<const string_view, const request_t, cmp_str> str_to_request_t;

extern std::map<const string_view, const httpversion, cmp_str> str_to_httpversion;
extern std::map<const httpversion, const string_view> httpversion_to_str;

extern std::map<string_view, string_view, cmp_str> ext_to_mime;

string_view get_mime(string_view ext);

#endif