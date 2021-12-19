#include "dynamic.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int DynamicPagesHandler::init(const char *lib_path, ConnectionSet &server) {
    handle = dlopen(lib_path, RTLD_LAZY);
    int (*dlinit)(ConnectionSet &server);

    dlerror();
    *(void **)(&dlinit) = dlsym(handle, "dynamic_init");
    char *err = dlerror();

    if (err == nullptr) return 1;

    *(void **)(&dlresolve) = dlsym(handle, "dynamic_resolve");
    err = dlerror();

    if (err == nullptr) return 1;

    return dlinit(server);
}

int DynamicPagesHandler::resolve(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self) {
    return dlresolve(response, data, request, self);
}

int DynamicPagesHandler::close() {
    int (*dynamic_close)();

    dlerror();
    *(void **)(&dynamic_close) = dlsym(handle, "dynamic_close");
    char *err = dlerror();

    if (err == nullptr) return 1;

    return dynamic_close();
}