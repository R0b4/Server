#include "Net/core.hpp"
#include "Net/socket.hpp"
#include "Http/handler.hpp"

void HandleRequest(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self) {
    response.version = httpversion_to_str[str_to_httpversion[string_view(request.version, data.buffer)]];
    response.status = string_view("200 OK", 6);

    string_view body("<center><h1>404 Not found</h1><p>Could not find this file.");

    char *str;
    response.add_header(HttpResponseHeader(hcontenttype, string_view("text/plain; charset=iso-8859-1")));

    if (!response.set_send_file(response.add_to_free(string_view(request.path + 1, data.buffer).make_c_str()), 1000000, true)) {
        response.add_header(HttpResponseHeader::make_content_length(body.size, str));
        response.set_send_buffer(body);
    }
}

int main(){
    ConnectionSet connections(1000);

    ConnectionFunctions funcs = HttpHandler::get<HandleRequest>();
    SocketFunctions sock_funcs = StandardSocket::get();

    connections.start_listener("1234", 30, &funcs, &sock_funcs);

    for (;;) connections.handle();
}