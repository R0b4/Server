#include "../Net/core.hpp"
#include "types.hpp"
#include "utils.hpp"

#define HTTP_BUFFER_SIZE 1000
#define HTTP_REALLOC_THESHOLD 500

/*
sources:
https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
https://nl.wikipedia.org/wiki/Hypertext_Transfer_Protocol
https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
*/

namespace HttpHandler {

    struct HttpData {
        HttpRequest request;

        char *buffer;
        size_t size;
        size_t parsed;
        size_t written;
    };

    void init(ConnectionSet *all, ConnectionHandler *self) {
        HttpData *data = new HttpData;
        data->buffer = (char *)malloc(HTTP_BUFFER_SIZE);
        data->size = HTTP_BUFFER_SIZE;
        data->written = 0;
        self->data = data;
    }

    void reallocate(HttpData &data){
        char *newbuffer = (char *)realloc(data.buffer, (data.size *= 2));
        free(data.buffer);
        data.buffer = newbuffer;
    }

    size_t get_receive_buffer(ConnectionHandler *self, char **buffer) {
        HttpData &data = self->get_data<HttpData>();

        size_t space = data.size - data.written;
        if (space < HTTP_REALLOC_THESHOLD) {
            reallocate(data);
            space = data.size - data.written;
        }

        *buffer = data.buffer + data.written;
        return space;
    }

    ssize_t parse_top(const char *buffer, string_pos to_parse, HttpRequest &request) {
        using namespace HttpChars;

        size_t first_space;
        if (!str_find(string_view(to_parse, buffer), space, first_space)){
            return -1;
        }

        size_t second_space;
        size_t second_space_offset = first_space + space.size;
        if (!str_find(string_view(to_parse, buffer) + second_space_offset, space, second_space)){
            return -1;
        }

        size_t crlf_pos;
        size_t crlf_offset = second_space_offset + second_space + space.size;
        if (!str_find(string_view(to_parse, buffer) + crlf_offset, crlf, crlf_pos)) {
            return -1;
        }

        request.type = str_to_request_t[string_view(string_pos(to_parse.start, first_space), buffer)];
        request.path = string_pos(to_parse.start + second_space_offset, second_space);
        request.version = str_to_httpversion[string_view(string_pos(to_parse.start + crlf_offset, crlf_pos), buffer)];

        return crlf_pos + crlf.size;
    }

    ssize_t parse_header(const char *buffer, string_pos to_parse, HttpRequest &request, bool &empty_line) {
        using namespace HttpChars;

        size_t colon_pos = 0;
        bool found_colon = str_find(string_view(to_parse, buffer), colon_space, colon_pos);

        size_t crlf_pos;
        size_t crlf_offset = colon_pos + found_colon * colon_space.size;
        if (!str_find(string_view(to_parse, buffer) + crlf_offset, crlf, crlf_pos)){
            return -1;
        }

        if (!found_colon) {
            empty_line = true;
            return crlf.size;
        }

        HttpHeader header;
        header.name = string_pos(to_parse.start, colon_pos);
        header.type = hother;
        if (str_to_header_t.count(string_view(header.name, buffer))) {
            header.type = str_to_header_t[string_view(header.name, buffer)];
        }
        header.value = string_pos(to_parse.start + crlf_offset, crlf_pos);

        if (header.type == hcontentlength) {
            string_view(header.value, buffer).get_num(request.content_length);
        }

        return crlf_pos + crlf.size;
    }

    void on_receive(ConnectionHandler *self, size_t received) {
        HttpData &data = self->get_data<HttpData>();
        HttpRequest &request = data.request;

        data.written += received;
        for (char *buffer = data.buffer + data.parsed; data.parsed < data.written; buffer = data.buffer + data.parsed) {
            if (request.progress == none) {

            } else if (request.progress == top) {

            } else {

            }   
        }
    }
}