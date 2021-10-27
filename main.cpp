#include "Net/core.hpp"
#include "Net/socket.hpp"
#include "Http/handler.hpp"

void HandleRequest(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self) {
    response.version = httpversion_to_str[str_to_httpversion[string_view(request.version, data.buffer)]];
    response.status = string_view("200 OK", 6);

    string_view body("<body><center><h1>Welcome</h1><p>This is a test page!</p></center></body>");

    char *str;
    response.add_header(HttpResponseHeader(hcontenttype, string_view("text/html; charset=iso-8859-1")));
    response.add_header(HttpResponseHeader::make_content_length(body.size, str));

    response.set_send_buffer(body);
}

namespace Simple {
    struct simple_data {
        constexpr static size_t standard_in_size = 1000;
        char *in;
    };

    void init(ConnectionSet *all, ConnectionHandler *self) {
        simple_data *data = new simple_data;
        data->in = (char *)malloc(simple_data::standard_in_size);
        self->data = data;
    }

    size_t get_receive_buffer(ConnectionHandler *self, char **buffer) {
        simple_data &data = self->get_data<simple_data>();
        *buffer = data.in;
        return simple_data::standard_in_size;
    }

    void on_receive(ConnectionHandler *self, size_t received) {
        simple_data &data = self->get_data<simple_data>();
        fwrite(data.in, 1, received, stdout);

        NetAction a;
        a.set_send(response_str, 10);
        self->register_action(a);

        a.set_send(response_str + 10, sizeof(response_str) - 10);
        self->register_action(a);

        a.set_close();
        self->register_action(a);
    }

	void on_ping(ConnectionHandler *self, int ping_data){

    }

    void close(ConnectionHandler *self) {
        simple_data &data = self->get_data<simple_data>();
        free(data.in);
    }

    ConnectionFunctions get(){
        ConnectionFunctions funcs;
        funcs.init = init;
        funcs.get_receive_buffer = get_receive_buffer;
        funcs.on_receive = on_receive;
        funcs.on_ping = on_ping;
        funcs.close = close;

        return funcs;
    }
}

int main(){
    ConnectionSet connections(1000);

    ConnectionFunctions funcs = HttpHandler::get<HandleRequest>();
    SocketFunctions sock_funcs = StandardSocket::get();

    connections.start_listener("1234", 30, &funcs, &sock_funcs);

    for (;;) connections.handle();  
}