#include <server/http/handler.hpp>

#define HTTP_BUFFER_SIZE 1000
#define HTTP_REALLOC_THESHOLD 500

namespace HttpHandler {
    size_t http_buffer_size;
    size_t http_realloc_threshold;

    void init(ConnectionSet &all, ConnectionHandler &self) {
        HttpData *data = new HttpData;
        data->buffer = (char *)malloc(http_buffer_size);
        data->size = http_buffer_size;
        self.data = data;
    }

    void reallocate(HttpData &data){
        void *newbuffer = malloc(data.size * 2);
        memcpy(newbuffer, data.buffer, data.size);
        free(data.buffer);
        data.buffer = (char *)newbuffer;
        data.size *= 2;
    }

    size_t get_receive_buffer(ConnectionHandler &self, char **buffer) {
        HttpData &data = self.get_data<HttpData>();

        size_t space = data.size - data.written;
        if (space < http_realloc_threshold) {
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

	void on_sent(ConnectionHandler &self){
        HttpData &data = self.get_data<HttpData>();
        if (data.pending_responses.size() == 0) return;

        HttpResponse &res = data.pending_responses.front();
        if (res.handle_send(&self)) {
            res.erase();
            data.pending_responses.pop();
        }
    }

    void close(ConnectionHandler &self) {
        HttpData &data = self.get_data<HttpData>();
        free(data.buffer);

        for (; !data.pending_responses.empty();) {
            data.pending_responses.front().erase();
            data.pending_responses.pop();
        }

        delete &data;
    }
}