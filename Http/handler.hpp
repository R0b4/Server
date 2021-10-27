#ifndef INCLUDE_HTTP_HANDLER
#define INCLUDE_HTTP_HANDLER

#include "../Net/core.hpp"
#include "tables.hpp"
#include "utils.hpp"
#include "request.hpp"
#include "response.hpp"

#define HTTP_BUFFER_SIZE 1000
#define HTTP_REALLOC_THESHOLD 500

/*
sources:
https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
https://nl.wikipedia.org/wiki/Hypertext_Transfer_Protocol
https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
*/

constexpr char response_str[] = "HTTP/1.1 200 OK\r\nDate: Thu, 08 Apr 2004 18:24:33 GMT\r\nContent-Type: text/html; charset=iso-8859-1\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\nHello world!";

namespace HttpHandler {

    struct HttpData {
        HttpRequest request;

        char *buffer;
        size_t size;
        size_t parsed;
        size_t written;

        std::queue<HttpResponse> pending_responses;

        inline HttpData() : written(0), parsed(0) {}
    };

    void init(ConnectionSet *all, ConnectionHandler *self) {
        HttpData *data = new HttpData;
        data->buffer = (char *)malloc(HTTP_BUFFER_SIZE);
        data->size = HTTP_BUFFER_SIZE;
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

        request.type = string_pos(to_parse.start, first_space);
        request.path = string_pos(to_parse.start + second_space_offset, second_space);
        request.version = string_pos(to_parse.start + crlf_offset, crlf_pos);

        return crlf_offset + crlf_pos + crlf.size;
    }

    ssize_t parse_header(const char *buffer, string_pos to_parse, HttpRequest &request, bool &empty_line) {
        using namespace HttpChars;

        size_t colon_pos = 0;
        bool found_colon = str_find(string_view(to_parse, buffer), colon, colon_pos);

        size_t crlf_pos;
        size_t crlf_offset = colon_pos + found_colon * colon.size;
        if (!str_find(string_view(to_parse, buffer) + crlf_offset, crlf, crlf_pos)){
            return -1;
        }

        if (!found_colon) {
            empty_line = true;
            return crlf.size;
        }

        HttpRequestHeader header;
        header.name = string_pos(to_parse.start, colon_pos);
        header.value = string_pos(to_parse.start + crlf_offset, crlf_pos);

        if (str_to_header_t.count(string_view(header.name, buffer))) {
            header.type = str_to_header_t[string_view(header.name, buffer)];
        } else header.type = hother;

        if (header.type == hcontentlength) {
            string_view(header.value, buffer).get_num(request.content_length);
        }

        request.headers.push_back(header);

        return crlf_offset + crlf_pos + crlf.size;
    }

    template<void (*get_response)(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self)>
    void on_receive(ConnectionHandler *self, size_t received) {
        HttpData &data = self->get_data<HttpData>();
        HttpRequest &request = data.request;

        data.written += received;
        for (;;) {
            string_pos to_parse(data.parsed, data.written - data.parsed);
            ssize_t parsed = 0;

            if (request.progress == none) {
                if ((parsed = parse_top(data.buffer, to_parse, request)) == -1) return;

                request.progress = top;
            } else if (request.progress == top) {
                bool empty = false;
                if ((parsed = parse_header(data.buffer, to_parse, request, empty)) == -1) return;

                if (empty) request.progress = headers;
            } else if (request.progress == headers) {
                if (to_parse.size < request.content_length) return;

                request.body = string_pos(to_parse.start, request.content_length);
                request.progress = all;
            } else {
                data.pending_responses.push(HttpResponse());
                HttpResponse &res = data.pending_responses.front();
                get_response(res, data, request, self);

                if (data.pending_responses.size() == 1) {
                    if (res.handle_send(self)) {
                        res.erase();
                        data.pending_responses.pop();
                    }
                }

                data.written = 0;
                data.parsed = 0;
                request = HttpRequest();

                return;
            }

            data.parsed += parsed;
        }
    }

	void on_ping(ConnectionHandler *self, int ping_data){
        HttpData &data = self->get_data<HttpData>();
        if (data.pending_responses.size() == 0) return;

        HttpResponse &res = data.pending_responses.front();
        if (res.handle_send(self)) {
            res.erase();
            data.pending_responses.pop();
        }
    }

    void close(ConnectionHandler *self) {
        HttpData &data = self->get_data<HttpData>();
        free(data.buffer);
    }

    template<void (*get_response)(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self)>
    ConnectionFunctions get(){
        ConnectionFunctions funcs;
        funcs.init = init;
        funcs.get_receive_buffer = get_receive_buffer;
        funcs.on_receive = on_receive<get_response>;
        funcs.on_ping = on_ping;
        funcs.close = close;

        return funcs;
    }
}

#endif