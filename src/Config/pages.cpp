#ifndef INCLUDE_CONFIG_PAGES
#define INCLUDE_CONFIG_PAGES

#include "options.hpp"
#include "../compression/compress.hpp"

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

std::map<string_view, page_t, cmp_str> pages;
std::vector<void *> pages_to_free;

const char *slash = "/";
const char *static_pages_dir = "/pages";
const char *gzip_pages_dir = "/gzip";

const char *rel_cert_path = "/cert.pem";
const char *rel_pkey_path = "/key.pem";
const char *rel_opt_path = "/options.txt";
const char *rel_path_404page = "/404.html";

char path_404page[PATH_MAX];

const char *gzip_extension = ".gzip";

//source: https://codeforwin.org/2018/03/c-program-to-list-all-files-in-a-directory-recursively.html
bool get_files(const char *base_path, std::vector<char *> &files) {
    char path[PATH_MAX];
    struct dirent *dp;
    DIR *dir = opendir(base_path);

    if (!dir) return false;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            strcpy(path, base_path);
            strcat(path, "/");
            strcat(path, dp->d_name);

            if (!get_files(path, files)) {
                size_t size = strlen(path) + 1;
                char *f = (char *)malloc(size);
                
                memcpy(f, path, size);
                files.push_back(f);
            }
        }
    }

    closedir(dir);
    return true;
}

void pages_init(options opt, const char *root) {
    char path_buffer[PATH_MAX];

    std::vector<char *> static_pages;
    strcpy(path_buffer, root);
    strcat(path_buffer, static_pages_dir);
    get_files(path_buffer, static_pages);
    size_t const_static_path_len = strlen(path_buffer);

    char gzip_path[PATH_MAX];
    strcpy(gzip_path, root);
    strcat(gzip_path, gzip_pages_dir);
    size_t const_gzip_path_len = strlen(gzip_path);

    strcpy(path_404page, root);
    strcat(path_404page, rel_path_404page);

    std::set<string_view, cmp_str> gzip_path_set;
    for (char *s : opt.gzip_paths) {
        gzip_path_set.insert(string_view(s));
    }

    for (char *str : static_pages) {
        page_t p;
        p.filepath = str;
        p.gzipped = false;

        size_t str_len = strlen(str);

        char *path = (char *)malloc(str_len - const_static_path_len + sizeof(char));
        strcpy(path, str + const_static_path_len);

        string_view key(path);

        if (gzip_path_set.find(key) != gzip_path_set.end()) {
            const size_t gzip_ext_len = 5 * sizeof(char);

            p.gzipped = true;
            char *gzip_str = (char *)malloc(const_gzip_path_len + strlen(path) + gzip_ext_len + sizeof(char));
            strcpy(gzip_str, gzip_path);
            strcat(gzip_str, path);
            strcat(gzip_str, gzip_extension);

            p.filepath = gzip_str;

            pages_to_free.push_back(gzip_str);
        }

        pages[key] = p;

        pages_to_free.push_back(path);
        pages_to_free.push_back(str);
    }

    for (options::route r : opt.routes) {
        auto it = pages.find(string_view(r.path));
        if (it == pages.end()) continue;

        pages[r.alias] = it->second;
    }
}

void gzip_init(options &opt, const char *root) {
    char pages[PATH_MAX];
    strcpy(pages, root);
    strcat(pages, static_pages_dir);

    char gzip_path[PATH_MAX];
    strcpy(gzip_path, root);
    strcat(gzip_path, gzip_pages_dir);

    char *rel_pages = pages + strlen(pages);
    char *rel_gzip = gzip_path + strlen(gzip_path);

    for (char *p : opt.gzip_paths) {
        strcpy(rel_pages, p);
        strcpy(rel_gzip, p);
        strcat(rel_gzip, gzip_extension);

        FILE *page = fopen(pages, "rb");
        if (page == nullptr) continue;

        FILE *gz_page = fopen(gzip_path, "wb");
        if (gzip_path == nullptr) continue;

        gzip_compress(page, gz_page, Z_BEST_COMPRESSION);
    }
}

//https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
int create_dir(const char *path, mode_t mask){
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
        return 0;
    }

    return 1;
}

string_view get_extension(string_view path) {
    size_t pos;
    bool found = str_find(path, string_view("."), pos);

    if (found) return path + pos;
    else return path + path.size;
}

void create_file(const char *path, string_view content) {
    FILE *file = fopen(path, "wb");
    content.print(file);
    fflush(file);
    fclose(file);
}

#endif