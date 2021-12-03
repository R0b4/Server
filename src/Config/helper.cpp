#include "helper.hpp"

const char *help_prompt = R"(Command options:
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
(10) display all options
(11) save
(12) write
)";

void run_config_helper(const char *path_to_opts, options &opt) {
    printf("%s\n", help_prompt);
    for (;;) {
        
    }
}