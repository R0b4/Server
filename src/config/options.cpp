#include <server/config/options.hpp>

char *create_mallocd_str(const char *str) {
    char *s = (char *)malloc(strlen(str) + 1);
    strcpy(s, str);
    return s;
}

int options::read_options(FILE *file) {
    fscanf(file, "%lu %lu %lu\n", &send_chunk_size, &http_begin_buffer_size, &http_realloc_threshold);
    fscanf(file, "%i\n", &maxconnections);

    port = read_str<10>(file);
    fscanf(file, "%i\n", &backlog);
    char sslc;
    fscanf(file, "%hhi\n", &sslc);
    use_ssl = sslc;
    
    ssl_port = read_str<10>(file);
    fscanf(file, "%i\n", &ssl_backlog);

    char dynamic;
    fscanf(file, "%hhi\n", &dynamic);
    use_dynamic_pages = dynamic;

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
        //printf("a %s\n", p);
        gzip_paths.push_back(p);
    }

    return 0;
}

int options::write_options(FILE *file) const {
    fprintf(file, "%lu %lu %lu\n", send_chunk_size, http_begin_buffer_size, http_realloc_threshold);
    fprintf(file, "%i\n", maxconnections);

    fprintf(file, "%s\n%i\n", port, backlog);

    fprintf(file, "%hhi\n", use_ssl);
    fprintf(file, "%s\n%i\n", ssl_port, ssl_backlog);

    fprintf(file, "%hhi\n", use_dynamic_pages);

    fprintf(file, "%lu %lu\n", routes.size(), gzip_paths.size());

    for (route r : routes) {
        fprintf(file, "%s\n", r.alias);
        fprintf(file, "%s\n", r.path);
    }

    for (char *s : gzip_paths) {
        fprintf(file, "%s\n", s);
    }

    fflush(file);
    return 0;
}

void options::create_default_options() {
    send_chunk_size = 10000;
    http_begin_buffer_size = 1000;
    http_realloc_threshold = 500;
    maxconnections = 100;
    backlog = 100;

    port = create_mallocd_str("80");
    use_ssl = false;

    ssl_backlog = 100;
    ssl_port = create_mallocd_str("443");

    use_dynamic_pages = false;
}

void options::free_options() {
    free(port);
    free(ssl_port);

    for (route r : routes) {
        free(r.alias);
        free(r.path);
    }

    for (char *p : gzip_paths) {
        free(p);
    }

    *this = options();
}