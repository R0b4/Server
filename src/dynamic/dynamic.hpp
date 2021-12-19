#include "../Http/handler.hpp"

//https://linux.die.net/man/3/dlsym
struct DynamicPagesHandler {
    void *handle;
    int (*dlresolve)(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self);

    int init(const char *lib_path, ConnectionSet &server);
    int resolve(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self);
    int close();
};