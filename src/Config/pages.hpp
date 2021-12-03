#ifndef INCLUDE_CONFIG_PAGES
#define INCLUDE_CONFIG_PAGES

#include "options.hpp"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


struct page_t {
    const char *filepath;
    bool gzipped;
};

extern std::map<string_view, page_t, cmp_str> pages;
extern std::vector<void *> pages_to_free;

extern const char *slash;
extern const char *static_pages_dir;
extern const char *gzip_pages_dir;

extern const char *rel_cert_path;
extern const char *rel_pkey_path;
extern const char *rel_opt_path;
extern const char *rel_path_404page;

extern char path_404page[PATH_MAX];

extern const char *gzip_extension;

bool get_files(const char *base_path, std::vector<char *> &files);
void pages_init(options opt, const char *root);

int create_dir(const char *path, mode_t mask);
string_view get_extension(string_view path);
void create_file(const char *path, string_view content);

#endif