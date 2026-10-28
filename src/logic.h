#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include <string.h>

#include "hash.h"
#include "darray.h"

struct Pair {
    char* path;
    struct Hash hash;
};

struct Pairs {
    struct Pair *items;
    size_t count;
    size_t capacity;
};


bool hash_file(const char* path, struct Hash* out);

void reset_pairs();
struct Pairs get_pairs();

void hash_file_and_store(const char* path);

#endif // LOGIC_H
