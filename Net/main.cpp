#include "core.hpp"
#include "socket.hpp"

int read_file(const char *path, char *&buf) {
    FILE *f = fopen(path, "rb");
    fseek (f, 0, SEEK_END);
    int length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buf = (char *)malloc(length);
    if (buf)
    {
        fread (buf, 1, length, f);
    }
    fclose (f);

    return length;
}

constexpr char str[] = "HTTP/1.1 200 OK\r\nDate: Thu, 08 Apr 2004 18:24:33 GMT\r\nContent-Type: text/html; charset=iso-8859-1\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\nHello world!";

namespace Echo {
    constexpr int buffer_size = 100;

    struct EchoData {
        char *buffer;
        char *file;
        int file_length;
    };

    void init(ConnectionSet *all, ConnectionHandler *self) {
        EchoData *data = new EchoData;
        data->buffer = (char *)malloc(buffer_size);
        data->file_length = read_file("response.txt", data->file);
        self->data = data;

        NetAction action;
        action.set_send((char *)str, strlen(str));
        self->register_action(action);

        action.set_close();
        self->register_action(action);
    }

    size_t get_receive_buffer(ConnectionHandler *self, char **buffer) {
        *buffer = ((EchoData *)self->data)->buffer;
        return buffer_size;
    }

    void on_receive(ConnectionHandler *self, size_t received){
        
    }

	void on_ping(ConnectionHandler *self, int ping_data){

    }

	void close(ConnectionHandler *self) {
        free(((EchoData *)self->data)->buffer);
        free(((EchoData *)self->data)->file);
        free(self->data);
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

    ConnectionFunctions funcs = Echo::get();
    SocketFunctions sock_funcs = StandardSocket::get();

    connections.start_listener("1234", 30, &funcs, &sock_funcs);

    for (;;) {
        connections.handle();
    }
}