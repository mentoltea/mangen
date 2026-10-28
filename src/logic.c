#include "logic.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#define READ_CHUNK_SIZE (64 * 1024)

static struct Pairs pairs;

static bool read_file(const char* path, char** out_data, size_t* out_size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "mangen: cannot open file '%s': %s\n", path, strerror(errno));
        return false;
    }

    char* data = NULL;
    size_t size = 0;
    size_t capacity = 0;
    bool ok = true;

    for (;;) {
        if (size == capacity) {
            size_t new_capacity = capacity ? capacity * 2 : READ_CHUNK_SIZE;
            char* new_data = realloc(data, new_capacity);
            if (!new_data) {
                fprintf(stderr, "mangen: out of memory while reading '%s'\n", path);
                ok = false;
                break;
            }
            data = new_data;
            capacity = new_capacity;
        }

        size_t read = fread(data + size, 1, capacity - size, file);
        size += read;
        if (read == 0) break;
    }

    if (ok && ferror(file)) {
        fprintf(stderr, "mangen: error reading file '%s': %s\n", path, strerror(errno));
        ok = false;
    }
    fclose(file);

    if (!ok) {
        free(data);
        return false;
    }
    *out_data = data;
    *out_size = size;
    return true;
}

static bool try_hash_file(const char* path, uint32_t* out_hash) {
    char* data;
    size_t size;
    if (!read_file(path, &data, &size)) return false;

    *out_hash = hash32(data, size);
    free(data);
    return true;
}

uint32_t hash_file(const char* path) {
    uint32_t hash = 0;
    try_hash_file(path, &hash);
    return hash;
}

void reset_pairs() {
    for (size_t i=0; i<pairs.count; i++) {
        free(pairs.items[i].path);
    }
    da_free(pairs);
}

struct Pairs get_pairs() {
    return pairs;
}

void hash_file_and_store(const char* path) {
    uint32_t hash;
    if (!try_hash_file(path, &hash)) return;

    char* path_copy = strdup(path);
    if (!path_copy) {
        fprintf(stderr, "mangen: out of memory\n");
        return;
    }

    struct Pair pair = {.path = path_copy, .hash = hash};
    da_append(pairs, pair);
}
