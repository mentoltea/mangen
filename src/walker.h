#ifndef WALKER_H
#define WALKER_H

#include <stdbool.h>

// void resursive_walk(const char* path, void (*callback)(const char*));
void iterarive_walk(const char* path, void (*callback)(const char*));

bool is_directory(const char* path);
bool is_regular_file(const char* path);

#endif // WALKER_H