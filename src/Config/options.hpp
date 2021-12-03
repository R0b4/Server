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

    int read_options(FILE *file) {
        fscanf(file, "%lu %lu %lu\n", &send_chunk_size, &http_begin_buffer_size, &http_realloc_threshold);
        fscanf(file, "%i\n", &maxconnections);

        port = read_str<10>(file);
        fscanf(file, "%i\n", &backlog);
        fscanf(file, "%hhi\n", &use_ssl);
        if (use_ssl) {
            ssl_port = read_str<10>(file);
            fscanf(file, "%i\n", &ssl_backlog);
        }

        size_t routes_size;
        size_t gzip_size;
        fscanf(file, "%lu %lu\n", &routes_size, &gzip_size);

        for (size_t i = 0; i < routes_size; i++) {
            route r;
            r.alias = read_str<PATH_MAX>(file);
            r.path = read_str<PATH_MAX>(file);
            routes.push_back(r);
        }

        for (size_t i = 0; i < gzip_size; i++) {
            char *p = read_str<PATH_MAX>(file);
            gzip_allocs.push_back(p);
            gzip_paths.insert(string_view(p));
        }

        return 0;
    }

    int write_options(FILE *file) const {
        fprintf(file, "%lu %lu %lu\n", send_chunk_size, http_begin_buffer_size, http_realloc_threshold);
        fprintf(file, "%i\n", maxconnections);

        fprintf(file, "%s\n%i\n", port, backlog);
        fprintf(file, "%hhi\n", use_ssl);
        if (use_ssl) {
            fprintf(file, "%s\n%i\n", ssl_port, ssl_backlog);
        }

        fprintf(file, "%lu %lu\n", routes.size(), gzip_paths.size());

        for (route r : routes) {
            fprintf(file, "%s\n", r.alias);
            fprintf(file, "%s\n", r.path);
        }

        for (string_view s : gzip_paths) {
            fprintf(file, "%s\n", s.str);
        }

        fflush(file);
        return 0;
    }

    void create_default_options() {
        send_chunk_size = 10000;
        http_begin_buffer_size = 1000;
        http_realloc_threshold = 500;
        maxconnections = 100;
        backlog = 100;

        port = create_mallocd_str("80");
        use_ssl = false;
    }

    void free_options() {
        free(port);
        if (use_ssl) free(ssl_port);

        for (route r : routes) {
            free(r.alias);
            free(r.path);
        }

        for (char *p : gzip_allocs) {
            free(p);
        }

        *this = options();
    }

private:
    template<size_t MAX>
    char *read_str(FILE *file) {
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

    char *create_mallocd_str(const char *str) {
        char *s = (char *)malloc(strlen(str) + 1);
        strcpy(s, str);
        return s;
    }
};



#endif
