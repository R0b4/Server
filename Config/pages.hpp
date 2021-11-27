#include "options.hpp"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

struct page_t {
    const char *filepath;
    bool gzipped;
};

std::map<string_view, page_t> pages;
std::vector<void *> pages_to_free;

inline page_t resolve_static_path(const string_view &path) {
    return pages[path];
}

void pages_init(options opt) {
    char path_buffer[PATH_MAX];

    std::vector<char *> static_pages;
    size_t const_static_path_len = strlen(opt.root) + strlen(opt.pages_dir);
    strcpy(path_buffer, opt.root);
    strcat(path_buffer, "/");
    strcat(path_buffer, opt.pages_dir);
    get_files(path_buffer, static_pages);

    std::vector<char *> gzip_pages;
    size_t const_static_path_len = strlen(opt.root) + strlen(opt.gzip_pages_dir);
    strcpy(path_buffer, opt.root);
    strcat(path_buffer, "/");
    strcat(path_buffer, opt.gzip_pages_dir);
    get_files(path_buffer, gzip_pages);

    for (char *str : static_pages) {
        page_t p;
        p.filepath = str;
        p.gzipped = false;

        char *path = (char *)malloc(strlen(str) - const_static_path_len);
        strcpy(path, str + const_static_path_len);

        pages[string_view(path)] = p;

        pages_to_free.push_back(path);
        pages_to_free.push_back(str);
    }

    
}

//source: https://codeforwin.org/2018/03/c-program-to-list-all-files-in-a-directory-recursively.html
bool get_files(const char *base_path, std::vector<char *> &files) {
    char path[PATH_MAX];
    const char *pathstr = path;
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