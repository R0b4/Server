#ifndef INCLUDE_CONFIG_OPTIONS
#define INCLUDE_CONFIG_OPTIONS

#include "../utils.hpp"
#include <map>
#include <vector>
#include <set>
#include <stack>
#include <limits.h>
#include <arpa/inet.h>

struct options {
    struct route {
        char *alias;
        char *path;
    };

    std::vector<route> routes;
    std::set<string_view, cmp_str> gzip_paths;
    std::vector<char *> gzip_allocs;

    size_t send_chunk_size;
    size_t http_begin_buffer_size;
    size_t http_realloc_threshold;

    int maxconnections;
    int backlog;

    char *port;
    
    bool use_ssl;
    int ssl_backlog;
    char *ssl_port;

    int read_options(FILE *file);
    int write_options(FILE *file) const;
    void create_default_options();
    void free_options();
};

#endif
