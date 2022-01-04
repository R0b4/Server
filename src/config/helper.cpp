#include <server/config/helper.hpp>

const static char *help_prompt = R"(Command options:
(0) display this prompt
(1) set send_chunk_size
(2) set http_begin_buffer_size
(3) set http_realloc_threshold
(4) set maxconnections
(5) set backlog
(6) set port
(7) set use_ssl
(8) set ssl_backlog
(9) set ssl_port
(10) add gzip path
(11) remove gzip path
(12) add route
(13) remove route
(14) display all options
(15) save
(16) exit
)";

static void display_help(const char *name, const char *scan_args, options &opt) {
    puts(help_prompt);
}

template<typename T, T options::*member>
static void set_num_option(const char *name, const char *scan_args, options &opt) {
    T val;

    printf("set %s: ", name);
    scanf(scan_args, &val);

    opt.*member = val;
}

template<size_t max_str_len, char *options::*member>
static void set_string_option(const char *name, const char *scan_args, options &opt) {
    char buffer[max_str_len];

    printf("set %s: ", name);
    fgets(buffer, max_str_len, stdin);

    printf("%s\n", buffer);

    size_t len = strlen(buffer);
    if (buffer[len - 1] == '\n') {
        len--;
        buffer[len] = '\0';
    }

    printf("%s\n", buffer);

    char *str = (char *)malloc(len + 1);
    strcpy(str, buffer);

    opt.*member = str;
}

static void set_usessl_options(const char *name, const char *scan_args, options &opt) {
    printf("set %s:\n (y/n): ", name);
    char c = fgetc(stdin);
    
    opt.use_ssl = c == 'y';
}

static void add_to_gzip(const char *name, const char *scan_args, options &opt) {
    puts("file to gzip: ");
    opt.gzip_paths.push_back(read_str<PATH_MAX>(stdin));
}

static void remove_from_gzip(const char *name, const char *scan_args, options &opt) {
    size_t i = 0;
    for (char *s : opt.gzip_paths) {
        printf("(%lu) %s\n", i, s);
        i++;
    }

    puts("enter index to remove: ");
    scanf("%lu", &i);

    if (i < opt.gzip_paths.size()) {
        opt.gzip_paths.erase(opt.gzip_paths.begin() + i);
    } else {
        printf("chosen index too big\n");
    }
}

static void add_to_routes(const char *name, const char *scan_args, options &opt) {
    options::route r;
    puts("alias: ");
    r.alias = read_str<PATH_MAX>(stdin);
    puts("file path: ");
    r.path = read_str<PATH_MAX>(stdin);
    opt.routes.push_back(r);
}

static void remove_from_routes(const char *name, const char *scan_args, options &opt) {
    size_t i = 0;
    for (options::route r : opt.routes) {
        printf("(%lu) %s -> %s\n", i, r.alias, r.path);
        i++;
    }

    puts("enter index to remove: ");
    scanf("%lu", &i);
    if (i < opt.routes.size()) {
        opt.routes.erase(std::next(opt.routes.begin(), i));
    } else {
        printf("chosen index too big\n");
    }
}

typedef void (*option_func_t)(const char *, const char *, options &);

const char *test = "aaa";

static void display_all_options(const char *name, const char *scan_args, options &opt) {
    printf("%s: %lu\n", "send_chunk_size", opt.send_chunk_size);
    printf("%s: %lu\n", "http_begin_buffer_size", opt.http_begin_buffer_size);
    printf("%s: %lu\n", "http_realloc_threshold", opt.http_realloc_threshold);
    printf("%s: %i\n", "maxconnections", opt.maxconnections);
    printf("%s: %i\n", "backlog", opt.backlog);
    printf("%s: %s\n", "port", opt.port);
    printf("%s: %s\n", "usessl", opt.use_ssl ? "true" : "false");
    printf("%s: %i\n", "ssl_backlog", opt.ssl_backlog);
    printf("%s: %s\n", "ssl_port", opt.ssl_port);

    printf("routes: \n");
    for (const options::route &r : opt.routes) {
        printf("\t%s -> %s\n", r.alias, r.path);
    }

    printf("gzip paths: \n");
    for (const char *str : opt.gzip_paths) {
        printf("\t%s\n", str);
    }
};

constexpr unsigned int option_num = 17;
constexpr option_func_t option_func_array[option_num] = {
    display_help, 
    set_num_option<size_t, &options::send_chunk_size>, 
    set_num_option<size_t, &options::http_begin_buffer_size>,
    set_num_option<size_t, &options::http_realloc_threshold>,
    set_num_option<int, &options::maxconnections>,
    set_num_option<int, &options::backlog>,
    set_string_option<10, &options::port>,
    set_usessl_options,
    set_num_option<int, &options::ssl_backlog>,
    set_string_option<10, &options::ssl_port>,
    add_to_gzip,
    remove_from_gzip,
    add_to_routes,
    remove_from_routes,
    display_all_options,
    nullptr,
    nullptr
};

constexpr const char *option_names[option_num] = {
    nullptr,
    "send_chunk_size",
    "http_begin_buffer_size", 
    "http_realloc_threshold",
    "maxconnections",
    "backlog",
    "port",
    "use_ssl"
    "ssl_backlog",
    "ssl_port",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

constexpr unsigned int amount_typeenum = 4;
enum typeenum {
    enull, echar, eint, esizet
};

constexpr const char *type_to_printf_str[amount_typeenum] = {
    nullptr, "%c", "%i", "%lu"
};

constexpr typeenum type_for_func[option_num] = {
    enull, esizet, esizet, esizet, eint, eint, enull, enull, eint, 
    enull, enull, enull, enull, enull, enull, enull, enull
};

static void call_option_func(unsigned int opcode, options &opt) {
    option_func_array[opcode](option_names[opcode], 
        type_to_printf_str[type_for_func[opcode]], opt);
}

static void clear_stdin() {
    for (char c; (c = fgetc(stdin)) != EOF && c != '\n';);
}

void run_config_helper(const char *path_to_opts, options &opt) {
    call_option_func(0, opt);
    for (;;) {
        unsigned int option = ~0U;
        printf("\noperation number (0 for help): ");
        scanf("%u", &option);

        clear_stdin();

        if (option > 16) {
            printf("op number has to be between 0 and %u.\n", option_num);
            call_option_func(0, opt);
        }

        if (option == 15) {
            FILE *file = fopen(path_to_opts, "wb");
            opt.write_options(file);
            fclose(file);
        } else if (option == 16) break;
        else call_option_func(option, opt);
    }
}