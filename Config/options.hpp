#include "../utils.hpp"
#include <map>
#include <vector>
#include <set>
#include <stack>
#include <limits.h>

enum option_t {
    root_dir, rel_static, rel_gzip, gzip_pages, route, use_ssl
};

std::map<const string_view, const option_t, cmp_str> option_strings = {
    {string_view("root_dir"), root_dir},
    {string_view("rel_static"), rel_static},
    {string_view("gzip_pages"), gzip_pages},
    {string_view("route"), route},
    {string_view("use_ssl"), use_ssl}
};

struct options {
    struct route {
        const char *alias;
        const char *path;
    };

    const char *root;
    const char *pages_dir;
    const char *gzip_pages_dir;

    std::vector<const char *> gzip_paths;
    std::vector<route> routes;

    bool use_ssl;
};