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


bool match_pattern(const char* pattern, const char* str);

void set_excludes(char** patterns, size_t count);

bool is_excluded(const char* name);

#endif // LOGIC_H
