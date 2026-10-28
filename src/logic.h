#ifndef LOGIC_H
#define LOGIC_H

#include <string.h>

#include "hash.h"
#include "darray.h"

struct Pair {
    char* path;
    uint32_t hash;
};

struct Pairs {
    struct Pair *items;
    size_t count;
    size_t capacity;
};

uint32_t hash_file(const char* path);


void reset_pairs();
struct Pairs get_pairs();

void hash_file_and_store(const char* path);

#endif // LOGIC_H