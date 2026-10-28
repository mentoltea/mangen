#include "cli.h"
#include <stdio.h>
#include <string.h>

struct cli parse_arguments(int argc, char** argv) {
    struct cli result = {0};

    bool exclude_flag = false;
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') {
            if (exclude_flag) {
                da_append(result.excludes, argv[i]);
                exclude_flag = false;
                continue;
            }

            if (result.dir_path != NULL) {
                fprintf(stderr, "Dir path provided twice: %s and %s\n", result.dir_path, argv[i]);
                return result;
            }
            result.dir_path = argv[i];
            continue;
        }

        if (exclude_flag) {
            fprintf(stderr, "Expected exclude value, got flag: %s\n", argv[i]);
            return result;
        }

        // flags
        if (strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
            result.help = true;   
        } else
        if (strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--version")==0) {
            result.version = true;
        } else
        if (strcmp(argv[i], "-e")==0 || strcmp(argv[i], "--exclude")==0) {
            exclude_flag = true;
        } else {
            fprintf(stderr, "Unknown flag: %s\n", argv[i]);
            return result;
        }
    }

    if (exclude_flag) {
        fprintf(stderr, "Expected exclude value\n");
        return result;
    }

    result.valid = true;
    return result;
}