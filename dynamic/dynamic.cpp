#include <server/dynamic.hpp>

int dynamic_init(ConnectionSet &connections) {
    return 0;
}

int dynamic_handle(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self) {
    return 1;
}

bool dynamic_should_shutdown() {
    return false;
}

int dynamic_close() {
    return 0;
}