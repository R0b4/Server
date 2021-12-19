#include "Net/core.hpp"
#include "Net/socket.hpp"
#include "Http/handler.hpp"
#include "Config/options.hpp"
#include "Config/pages.hpp"
#include "Config/helper.hpp"
#include "dynamic/dynamic.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

options server_options;
DynamicPagesHandler dhandler;

const char *root_folder;

void make_404_response(HttpResponse &res) {
    char *buff;
    res.add_header(HttpResponseHeader::make_content_type("text/html", "charset=UTF-8", buff));
    res.add_to_free(buff);

    res.status = string_view("404 Not Found");
    res.set_send_file(path_404page, server_options.send_chunk_size, true);
}

void HandleRequest(HttpResponse &response, const HttpHandler::HttpData &data, const HttpRequest &request, const ConnectionHandler *self) {
    response.version = httpversion_to_str[str_to_httpversion[string_view(request.version, data.buffer)]];

    string_view path = string_view(request.path, data.buffer);

    if (server_options.use_dynamic_pages) {
        if (dhandler.resolve(response, data, request, self) == 0) return;
    }

    auto it = pages.find(path);
    if (it == pages.end()) {
        make_404_response(response);
    } else {
        page_t page = it->second;
        if (!response.set_send_file(page.filepath, server_options.send_chunk_size, true)) {
            make_404_response(response);
            return;
        }

        response.status = string_view("202 OK");

        char *buff;
        string_view mime = get_mime(get_extension(path));
        response.add_header(HttpResponseHeader::make_content_type(mime, "charset=UTF-8", buff));
        response.add_to_free(buff);

        if (page.gzipped) {
            response.add_header(HttpResponseHeader(hcontentenc, "gzip"));
        }
    }
}

enum command_t {
    crun, cconfig, ccreate, cgzip
};

std::map<string_view, command_t, cmp_str_case_insensitive> commands = {
    {string_view("run"), crun}, 
    {string_view("config"), cconfig}, 
    {string_view("create"), ccreate},
    {"gzip", cgzip}
};

int init_dir(const char *root) {
    char opt_path[PATH_MAX];
    strcpy(opt_path, root);
    strcat(opt_path, rel_opt_path);

    FILE *fopt = fopen(opt_path, "rb");
    if (!fopt) {
        printf("File %s doesn't exist.\n", opt_path);
        return 1;
    } else {
        if (server_options.read_options(fopt)) {
            printf("File %s was corrupted.\n", opt_path);
            return 1;
        }
        fclose(fopt);
    }


    pages_init(server_options, root);
    return 0;
}

void run_server() {
    ConnectionSet connections(server_options.maxconnections);

    ConnectionFunctions funcs = HttpHandler::get<HandleRequest>();
    SocketFunctions sock_funcs = StandardSocket::get();

    connections.start_listener(server_options.port, server_options.backlog, &funcs, &sock_funcs);

    HttpHandler::http_buffer_size = server_options.http_begin_buffer_size;
    HttpHandler::http_realloc_threshold = server_options.http_realloc_threshold;
    
    SocketFunctions ssl_sock_funcs;
    if (server_options.use_ssl) {
        char cert_path[PATH_MAX];
        strcpy(cert_path, root_folder);
        strcat(cert_path, rel_cert_path);
        
        char pkey_path[PATH_MAX];
        strcpy(pkey_path, root_folder);
        strcat(pkey_path, rel_pkey_path);

        ssl_sock_funcs = SSLSocket::get(SSLSocket::tls, cert_path, pkey_path);
        connections.start_listener(server_options.ssl_port, server_options.backlog, &funcs, &ssl_sock_funcs);
    }

    for (;;) { 
        connections.handle();
    }
}

int create_website_dir() {
    DIR *dir = opendir(root_folder);

    if (dir) closedir(dir);
    else return 1;

    mode_t mask = umask(0);

    char path[PATH_MAX];
    strcpy(path, root_folder);
    char *rel_part = path + strlen(path);
    
    strcpy(rel_part, static_pages_dir);
    create_dir(path, mask);
    strcpy(rel_part, gzip_pages_dir);
    create_dir(path, mask);

    strcpy(rel_part, rel_path_404page);
    create_file(path, string_view("<center><h1>404</h1><p>Page not found</p></center>"));

    strcpy(rel_part, rel_cert_path);
    create_file(path, string_view("#Paste your ssl certificate here"));

    strcpy(rel_part, rel_pkey_path);
    create_file(path, string_view("#Paste your ssl private key here."));

    server_options.create_default_options();
    strcpy(rel_part, rel_opt_path);
    FILE *opt_file = fopen(path, "wb");
    server_options.write_options(opt_file);
    fclose(opt_file);

    printf("Succesfully created\n");

    return 0;
}

int main(int argc, char **argv){
    if (argc > 1) {
        
        auto it = commands.find(string_view(argv[1]));
        if (it == commands.end() && false) {
            goto query_syntax_error;
        }

        command_t c = it->second;

        if (c == crun){
            if (argc > 2) {
                root_folder = argv[2];
                if (init_dir(root_folder)) return 1;

                run_server();
            } else {
                goto query_syntax_error;
            }
        } else if (c == cconfig) {
            if (argc > 2) {
                root_folder = argv[2];
                if (init_dir(root_folder)) return 1;

                char path[PATH_MAX];
                strcpy(path, root_folder);
                strcat(path, rel_opt_path);

                run_config_helper(path, server_options);
            } else {
                goto query_syntax_error;
            }
        } else if (c == ccreate) {
            if (argc > 2) {
                root_folder = argv[2];

                create_website_dir();
            } else {
                goto query_syntax_error;
            }
        } else if (c == cgzip) {
            if (argc > 2) {
                root_folder = argv[2];
                if (init_dir(root_folder)) return 1;

                gzip_init(server_options, root_folder);
            } else {
                goto query_syntax_error;
            }
        }
    }

    query_syntax_error:
    return 0;
}