#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "Options/strings.hpp"

int main()
{
    std::vector<char *> files;
    get_files("/home/rob", files);

    for (auto &t : files) {
        printf("%s\n", t);
    }

    return 0;
}