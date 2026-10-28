#ifndef CLI_H
#define CLI_H

#include "darray.h"
#include <stdbool.h>

struct Excludes {
    char **items;
    size_t count;
    size_t capacity;
};

struct cli {
    bool valid;
    const char* dir_path;
    
    struct Excludes excludes;
    bool help;
    bool version;
};

struct cli parse_arguments(int argc, char** argv);

#endif // CLI_H