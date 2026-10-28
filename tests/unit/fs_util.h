// File system helpers for unit tests that need real files.
// Each such test gets a fresh empty work directory as argv[1]
// (created by CTest, see tests/CMakeLists.txt) and chdir()s into it.

#ifndef FS_UTIL_H
#define FS_UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define fs_mkdir(path) _mkdir(path)
#define fs_chdir(path) _chdir(path)
#else
#include <unistd.h>
#define fs_mkdir(path) mkdir((path), 0755)
#define fs_chdir(path) chdir(path)
#endif

static bool make_dir(const char* path) {
    if (fs_mkdir(path) != 0) {
        perror(path);
        return false;
    }
    return true;
}

static bool write_file(const char* path, const void* data, size_t size) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        perror(path);
        return false;
    }
    bool ok = fwrite(data, 1, size, file) == size;
    return fclose(file) == 0 && ok;
}

static bool write_text(const char* path, const char* text) {
    return write_file(path, text, strlen(text));
}

static bool enter_work_dir(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s WORK_DIR\n", argv[0]);
        return false;
    }
    if (fs_chdir(argv[1]) != 0) {
        perror(argv[1]);
        return false;
    }
    return true;
}

#endif // FS_UTIL_H
