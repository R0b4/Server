#ifndef INCLUDE_HTTP_REQUEST
#define INCLUDE_HTTP_REQUEST

#include "tables.hpp"
#include "../utils.hpp"

#include <vector>

struct HttpRequestHeader {
    standard_header type;
    string_pos name;
    string_pos value;
};

enum progress_t {
    none, top, headers, all
};

struct HttpRequest {
    string_pos type;
    string_pos version;
    string_pos path;

    std::vector<HttpRequestHeader> headers;

    size_t content_length;
    string_pos body;

    progress_t progress;

    inline HttpRequest() : content_length(0), progress(none) {}
};

#endif