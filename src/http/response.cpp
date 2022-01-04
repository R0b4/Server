#include <server/http/response.hpp>

bool HttpResponse::set_send_file(const char *filename, size_t chunk_size, bool add_content_len_header) {
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

bool HttpResponse::set_send_buffer(string_view buffer) {
    send_file = false;
    sent_body = false;
    body = buffer.str;
    body_size = buffer.size;

    return true;
}

size_t HttpResponse::get_header_size(){
    size_t size = const_size + version.size + status.size;
    for (HttpResponseHeader &h : headers) {
        size += h.name.size + h.value.size;
    }

    size += headers.size() * const_per_header;
    return size;
}

void HttpResponse::serialize_headers(char *str){
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

void HttpResponse::add_send_and_ping(ConnectionHandler *handler, const char *str, size_t size, int ping) {
    handler->add_sent(string_view(str, size));
}

bool HttpResponse::handle_send(ConnectionHandler *handler) {
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

        add_send_and_ping(handler, buffer, buff_size, 0);
    } else {
        if (sent_body) return true;

        add_send_and_ping(handler, body, body_size, 0);
        sent_body = true;
    }

    return false;
}

void HttpResponse::erase() {
    for (void *p : to_free) {
        free(p);
    }
    if (send_file) fclose(file);
}