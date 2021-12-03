#ifndef INCLUDE_HTTP_HANDLER
#define INCLUDE_HTTP_HANDLER

#include "../Net/core.hpp"
#include "tables.hpp"
#include "../utils.hpp"
#include "request.hpp"
#include "response.hpp"

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

        std::queue<HttpResponse> pending_responses;

        inline HttpData() : written(0), parsed(0) {}
    };

    void init(ConnectionSet &all, ConnectionHandler &self);

    void reallocate(HttpData &data);

    size_t get_receive_buffer(ConnectionHandler &self, char **buffer);

    ssize_t parse_top(const char *buffer, string_pos to_parse, HttpRequest &request);

    ssize_t parse_header(const char *buffer, string_pos to_parse, HttpRequest &request, bool &empty_line);

    template<void (*get_response)(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self)>
    inline void on_receive(ConnectionHandler &self, size_t received) {
        HttpData &data = self.get_data<HttpData>();
        HttpRequest &request = data.request;

        data.written += received;
        for (;;) {
            string_pos to_parse(data.parsed, data.written - data.parsed);
            ssize_t parsed = 0;
            int klklkl = 1;

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
                HttpResponse res;
                get_response(res, data, request, &self);
                data.pending_responses.push(res);

                if (data.pending_responses.size() == 1) {
                    HttpResponse &resin = data.pending_responses.front();
                    if (resin.handle_send(&self)) {
                        resin.erase();
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

	void on_sent(ConnectionHandler &self);

    void close(ConnectionHandler &self);

    template<void (*get_response)(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self)>
    inline ConnectionFunctions get(){
        ConnectionFunctions funcs;
        funcs.init = init;
        funcs.get_receive_buffer = get_receive_buffer;
        funcs.on_receive = on_receive<get_response>;
        funcs.on_sent = on_sent;
        funcs.close = close;

        return funcs;
    }
}

#endif