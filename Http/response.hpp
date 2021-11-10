#ifndef INCLUDE_HTTP_RESPONSE
#define INCLUDE_HTTP_RESPONSE

#include "../utils.hpp"
#include "tables.hpp"
#include <math.h>
#include "../Net/core.hpp"

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
};

struct HttpResponse {
private:
    int id;
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

    inline HttpResponse() : send_file(false), not_sent_headers(true), id(rand()) {}

    template<typename T>
    inline T *add_to_free(T *str) {
        to_free.push_back(str);
        return str;
    }

    inline void add_header(HttpResponseHeader header) {
        headers.push_back(header);
    }

    bool set_send_file(const char *filename, size_t chunk_size, bool add_content_len_header) {
        send_file = true;
        file = fopen(filename, "rb");
        if (!file) return false;

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        sent = 0;

        this->chunk_size = chunk_size;

        if (add_content_len_header) {
            char *n_str;
            add_header(HttpResponseHeader::make_content_length(file_size, n_str));
            add_to_free(n_str);
        }

        return true;
    }

    bool set_send_buffer(string_view buffer) {
        send_file = false;
        sent_body = false;
        body = buffer.str;
        body_size = buffer.size;

        return true;
    }

    size_t get_header_size(){
        size_t size = const_size + version.size + status.size;
        for (HttpResponseHeader &h : headers) {
            size += h.name.size + h.value.size;
        }

        size += headers.size() * const_per_header;
        return size;
    }

    void serialize_headers(char *str){
        str = version.cpy(str);
        str = HttpChars::space.cpy(str);
        str = status.cpy(str);
        str = HttpChars::crlf.cpy(str);

        for (HttpResponseHeader &h : headers) {
            str = h.name.cpy(str);
            str = HttpChars::colon_space.cpy(str);
            str = h.value.cpy(str);
            str = HttpChars::crlf.cpy(str);
        }

        HttpChars::crlf.cpy(str);
    }

    void add_send_and_ping(ConnectionHandler *handler, const char *str, size_t size, int ping) {
        handler->add_sent(string_view(str, size));
    }

    bool handle_send(ConnectionHandler *handler) {
        if (not_sent_headers) {
            buff_size = get_header_size();
            buffer = (char *)malloc(buff_size + 10);
            add_to_free(buffer);
            memset(buffer, 'A', buff_size);
            serialize_headers(buffer);

            add_send_and_ping(handler, buffer, buff_size, 0);

            not_sent_headers = false;
        } else if (send_file) {
            if (sent == file_size) return true;

            if (buff_size != chunk_size) {
                buffer = (char *)malloc(chunk_size);
                add_to_free(buffer);
                buff_size = chunk_size;
            }

            int read = fread(buffer, 1, buff_size, file);
            sent += read;

            printf("response %i: sent %lu/%lu (%f%%)\n", id, sent, file_size, (float)sent * 100.0f / (float)file_size);

            add_send_and_ping(handler, buffer, buff_size, 0);
        } else {
            if (sent_body) return true;

            add_send_and_ping(handler, body, body_size, 0);
            sent_body = true;
        }

        return false;
    }

    void erase() {
        for (void *p : to_free) {
            free(p);
        }
    }
};

#endif