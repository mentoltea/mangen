#include <stdio.h>

#include "cli.h"
#include "logic.h"
#include "version.h"
#include "walker.h"

void help(FILE* stream) {
    fprintf(stream, "Usage: mangen [DIR_PATH] [OPTIONS]\n");
    fprintf(stream, "\n");
    fprintf(stream, "Generate a manifest of DIR_PATH: every regular file found recursively\n");
    fprintf(stream, "is printed to stdout together with its hash, one per line:\n");
    fprintf(stream, "    <path relative to DIR_PATH> : <hash>\n");
    fprintf(stream, "If DIR_PATH is omitted, the current directory is used.\n");
    fprintf(stream, "\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "  -h, --help                 print this help and exit\n");
    fprintf(stream, "  -v, --version              print version information and exit\n");
    fprintf(stream, "  -e, --exclude FILE_NAME    skip all files and directories whose name\n");
    fprintf(stream, "                             matches FILE_NAME; may be given several times\n");
    fprintf(stream, "\n");
    fprintf(stream, "FILE_NAME is a pattern matched against the whole name:\n");
    fprintf(stream, "  .    matches any single character\n");
    fprintf(stream, "  *    matches any sequence of characters (including empty)\n");
    fprintf(stream, "\n");
    fprintf(stream, "Errors while accessing files and directories are reported to stderr.\n");
    fprintf(stream, "\n");
    fprintf(stream, "Examples:\n");
    fprintf(stream, "  mangen example\n");
    fprintf(stream, "  mangen example -e dir_2 -e '*.png'\n");
}

// GIT_COMMIT_HASH is passed by the build system -DGIT_COMMIT_HASH="..."
#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

void version(FILE* stream) {
    fprintf(stream, "mangen %d.%d.%d by %s\n", VERSION_MAJOR, VERSION_MIDDLE, VERSION_MINOR, VERSION_AUTHOR_STR);
    fprintf(stream, "commit %s\n", GIT_COMMIT_HASH);
}

void usage(FILE* stream) {
    fprintf(stream, "mangen [DIR_PATH] [OPTIONS]\n");
    fprintf(stream, "mangen -h for help\n");
}

static const char* relative_path(const char* path, const char* dir_path) {
    size_t len = strlen(dir_path);
    bool has_sep = len > 0 && (dir_path[len - 1] == '/'
#ifdef _WIN32
                               || dir_path[len - 1] == '\\'
#endif
                              );
    return path + len + (has_sep ? 0 : 1);
}

static void print_manifest(FILE* stream, struct Pairs pairs, const char* dir_path) {
    char hex[HASH_HEX_SIZE];
    for (size_t i = 0; i < pairs.count; i++) {
        hash_to_hex(&pairs.items[i].hash, hex);
        fprintf(stream, "%s : %s\n", relative_path(pairs.items[i].path, dir_path), hex);
    }
}

int main(int argc, char** argv) {
    struct cli cli = parse_arguments(argc, argv);
    
    if (!cli.valid) {
        usage(stderr);
        return 1;
    }

    if (cli.help) {
        help(stdout);
        return 0;
    }
    if (cli.version) {
        version(stdout);
        return 0;
    }

    if (!cli.dir_path) cli.dir_path = ".";

    if (!is_directory(cli.dir_path)) {
        fprintf(stderr, "mangen: '%s' is not a directory or is not accessible\n", cli.dir_path);
        da_free(cli.excludes);
        return 1;
    }

    set_excludes(cli.excludes.items, cli.excludes.count);
    iterarive_walk(cli.dir_path, hash_file_and_store, is_excluded);
    print_manifest(stdout, get_pairs(), cli.dir_path);

    reset_pairs();
    da_free(cli.excludes);
    return 0;
}