#define _POSIX_C_SOURCE 200809L

#include "logic.h"

#include <errno.h>
#include <stdio.h>

#define READ_CHUNK_SIZE (64 * 1024)

static struct Pairs pairs;

// The file is fed to the hash in chunks, so memory use does not depend
// on the file size.
bool hash_file(const char* path, struct Hash* out) {
    static char buffer[READ_CHUNK_SIZE];

    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "mangen: cannot open file '%s': %s\n", path, strerror(errno));
        return false;
    }

    struct HashCtx ctx;
    hash_init(&ctx);

    size_t read;
    while ((read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        hash_update(&ctx, buffer, read);
    }

    bool ok = !ferror(file);
    if (!ok) {
        fprintf(stderr, "mangen: error reading file '%s': %s\n", path, strerror(errno));
    }
    fclose(file);

    if (ok) hash_final(&ctx, out);
    return ok;
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
    struct Hash hash;
    if (!hash_file(path, &hash)) return;

    char* path_copy = strdup(path);
    if (!path_copy) {
        fprintf(stderr, "mangen: out of memory\n");
        return;
    }

    struct Pair pair = {.path = path_copy, .hash = hash};
    da_append(pairs, pair);
}
