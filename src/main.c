#include <stdio.h>

#include "cli.h"
#include "version.h"

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

void version(FILE* stream) {
    fprintf(stream, "mangen %d.%d.%d by %s\n", VERSION_MAJOR, VERSION_MIDDLE, VERSION_MINOR, VERSION_AUTHOR_STR);
}

void usage(FILE* stream) {
    fprintf(stream, "mangen [DIR_PATH] [OPTIONS]\n");
    fprintf(stream, "mangen -h for help\n");
}

int main(int argc, char** argv) {
    struct cli cli = parse_arguments(argc, argv);
    
    if (cli.help) {
        help(stdout);
        return 0;
    }
    if (cli.version) {
        version(stdout);
        return 0;
    }

    if (!cli.valid) {
        usage(stderr);
        return 1;
    }

    if (!cli.dir_path) cli.dir_path = ".";

    printf("mangen %s", cli.dir_path);
    for (size_t i=0; i<cli.excludes.count; i++) {
        printf(" -e %s", cli.excludes.items[i]);
    }
    printf("\n");

    return 0;
}