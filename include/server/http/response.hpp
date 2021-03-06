#ifndef INCLUDE_HTTP_RESPONSE
#define INCLUDE_HTTP_RESPONSE

#include <server/utils.hpp>
#include <server/http/request.hpp>
#include <server/net/core.hpp>

#include <math.h>
#include <vector>

struct HttpResponseHeader {
    string_view name;
    string_view value;

    inline HttpResponseHeader(string_view name, string_view value) 
        : name(name), value(value) {}

    inline HttpResponseHeader(standard_header name, string_view value) 
        : name(header_to_str[name]), value(value) {}

    static inline HttpResponseHeader make_content_length(size_t len, char *&buffer) {
        size_t digits = get_digits(len);
        buffer = (char *)malloc(digits);
        
        for (size_t n = len, i = 0; i < digits; i++) {
            buffer[digits - i - 1] = (n % 10) + '0';
            n /= 10;
        }

        return HttpResponseHeader(header_to_str[hcontentlength], string_view(buffer, digits));
    }

    static inline HttpResponseHeader make_content_type(string_view mime, string_view charset, char *&buffer) {
        string_view const_str("; charset=");
        size_t len = mime.size + const_str.size + charset.size;

        buffer = (char *)malloc(len);
        {
            char *t = mime.cpy(buffer);
            t = const_str.cpy(t);
            t = charset.cpy(t);
        }

        return HttpResponseHeader(hcontenttype, string_view(buffer, len));
    }
};

struct HttpResponse {
private:
    bool not_sent_headers;
    char *buffer;
    size_t buff_size;

public:
    string_view version;
    string_view status;

    std::vector<HttpResponseHeader> headers;

    constexpr static size_t const_per_header = HttpChars::crlf.size + HttpChars::colon_space.size;
    constexpr static size_t const_size = HttpChars::crlf.size * 2 + HttpChars::space.size;

    bool send_file;
    union {
        struct {
            const char *body;
            size_t body_size;
            bool sent_body;
        };
        struct {
            const char *filename;
            size_t file_size;
            size_t sent;
            size_t chunk_size;
            FILE *file;
        };
    };

    std::vector<void *> to_free;

    inline HttpResponse() : not_sent_headers(true), send_file(false) {}

    template<typename T>
    inline T *add_to_free(T *str) {
        to_free.push_back(str);
        return str;
    }

    inline void add_header(HttpResponseHeader header) {
        headers.push_back(header);
    }

    bool set_send_file(const char *filename, size_t chunk_size, bool add_content_len_header);
    bool set_send_buffer(string_view buffer);

    size_t get_header_size();

    void serialize_headers(char *str);
    void add_send_and_ping(ConnectionHandler *handler, const char *str, size_t size, int ping);

    bool handle_send(ConnectionHandler *handler);
    void erase();
};

#endif