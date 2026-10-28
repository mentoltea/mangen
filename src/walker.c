#define _POSIX_C_SOURCE 200809L

#include "walker.h"
#include "darray.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct Strings {
    char **items;
    size_t count;
    size_t capacity;
};

bool is_directory(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

bool is_regular_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

// symlinks to directories are not followed, otherwise a link pointing
// to one of its parents would make the walk infinite
static bool is_symlink(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) return false;
    return S_ISLNK(st.st_mode);
}

static bool is_separator(char c) {
#ifdef _WIN32
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

static char* join_path(const char* base, const char* name) {
    size_t base_len = strlen(base);
    size_t name_len = strlen(name);
    bool need_sep = base_len > 0 && !is_separator(base[base_len - 1]);

    char* result = malloc(base_len + need_sep + name_len + 1);
    if (!result) return NULL;

    memcpy(result, base, base_len);
    if (need_sep) result[base_len] = '/';
    memcpy(result + base_len + need_sep, name, name_len + 1);
    return result;
}


static int compare_strings(const void* a, const void* b) {
    return strcmp(*(char* const*)a, *(char* const*)b);
}

static void free_strings(struct Strings* strings) {
    for (size_t i = 0; i < strings->count; i++) {
        free(strings->items[i]);
    }
    da_free(*strings);
}

// Reads names of all entries of dir_path (except "." and "..") into names,
// sorted so that the output does not depend on the file system order.
// Returns false and prints an error if the directory cannot be read.
static bool read_dir_sorted(const char* dir_path, struct Strings* names) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "mangen: cannot open directory '%s': %s\n", dir_path, strerror(errno));
        return false;
    }

    struct dirent* entry;
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char* name = strdup(entry->d_name);
        if (!name) {
            fprintf(stderr, "mangen: out of memory\n");
            break;
        }
        da_append(*names, name);
        errno = 0;
    }
    if (errno != 0) {
        fprintf(stderr, "mangen: error reading directory '%s': %s\n", dir_path, strerror(errno));
    }
    closedir(dir);

    if (names->count > 0) {
        qsort(names->items, names->count, sizeof(*names->items), compare_strings);
    }
    return true;
}

// skip(name) gets the bare entry name (not the full path); if it returns true
// the entry is ignored, and for a directory its whole subtree is never read.
// skip may be NULL. The root path itself is never passed to skip.
void iterarive_walk(const char* path, void (*callback)(const char*), bool (*skip)(const char*)) {
    if (!is_directory(path)) {
        fprintf(stderr, "mangen: '%s' is not a directory or is not accessible\n", path);
        return;
    }

    struct Strings queue = {0};
    size_t head = 0;

    char* root = strdup(path);
    if (!root) {
        fprintf(stderr, "mangen: out of memory\n");
        return;
    }
    da_append(queue, root);

    while (head < queue.count) {
        const char* dir_path = queue.items[head++];

        struct Strings names = {0};
        if (!read_dir_sorted(dir_path, &names)) {
            free_strings(&names);
            continue;
        }

        struct Strings subdirs = {0};
        for (size_t i = 0; i < names.count; i++) {
            // checked before stat(), so excluded directories are not even opened
            if (skip && skip(names.items[i])) continue;

            char* full = join_path(dir_path, names.items[i]);
            if (!full) {
                fprintf(stderr, "mangen: out of memory\n");
                continue;
            }

            struct stat st;
            if (stat(full, &st) != 0) {
                fprintf(stderr, "mangen: cannot access '%s': %s\n", full, strerror(errno));
                free(full);
            } else if (S_ISREG(st.st_mode)) {
                callback(full);
                free(full);
            } else if (S_ISDIR(st.st_mode) && !is_symlink(full)) {
                da_append(subdirs, full);
            } else {
                // sockets, devices, fifos, symlinks to directories: skipped
                free(full);
            }
        }
        free_strings(&names);


        if (subdirs.count > 0) {
            da_concat(queue, subdirs);
        }
        da_free(subdirs);
    }

    free_strings(&queue);
}
