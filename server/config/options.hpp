#ifndef INCLUDE_CONFIG_OPTIONS
#define INCLUDE_CONFIG_OPTIONS

#include <server/utils.hpp>

#include <map>
#include <vector>
#include <set>
#include <stack>
#include <limits.h>
#include <arpa/inet.h>

template<size_t MAX>
inline char *read_str(FILE *file) {
    char buffer[MAX];
    fgets(buffer, MAX, file);

    size_t len = strlen(buffer);
    if (buffer[len - 1] == '\n') {
        len--;
        buffer[len] = '\0';
    }

    char *str = (char *)malloc(len + 1);
    strcpy(str, buffer);
    return str;
}

char *create_mallocd_str(const char *str);

struct options {
    struct route {
        char *alias;
        char *path;
    };

    std::vector<route> routes;
    std::vector<char *> gzip_paths;

    size_t send_chunk_size;
    size_t http_begin_buffer_size;
    size_t http_realloc_threshold;

    int maxconnections;
    int backlog;

    char *port;
    
    bool use_ssl;
    int ssl_backlog;
    char *ssl_port;

    bool use_dynamic_pages;

    int read_options(FILE *file);
    int write_options(FILE *file) const;
    void create_default_options();
    void free_options();
};

#endif
