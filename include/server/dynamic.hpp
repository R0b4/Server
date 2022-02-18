#ifndef INCLUDE_DYNAMIC
#define INCLUDE_DYNAMIC

#include <server/net/core.hpp>
#include <server/http/handler.hpp>

int dynamic_init(ConnectionSet &connections);
int dynamic_handle(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self);
bool dynamic_should_shutdown();
int dynamic_close();

#endif