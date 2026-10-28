// Directory traversal: order of reported files, path building, the skip hook,
// non-regular files and symlinks.

// symlink(), mkfifo() and strdup() are POSIX
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>

#include "fs_util.h"
#include "test.h"
#include "walker.h"

// Everything the walker reports is collected here for checking.
#define MAX_SEEN 64
static char* seen[MAX_SEEN];
static size_t seen_count = 0;

static char* skip_calls[MAX_SEEN];
static size_t skip_count = 0;
static const char* skip_name = NULL;   // name the skip hook rejects

static void collect(const char* path) {
    if (seen_count < MAX_SEEN) seen[seen_count] = strdup(path);
    seen_count++;
}

static bool skip_hook(const char* name) {
    if (skip_count < MAX_SEEN) skip_calls[skip_count] = strdup(name);
    skip_count++;
    return skip_name && strcmp(name, skip_name) == 0;
}

static void reset_seen(void) {
    for (size_t i = 0; i < seen_count && i < MAX_SEEN; i++) free(seen[i]);
    for (size_t i = 0; i < skip_count && i < MAX_SEEN; i++) free(skip_calls[i]);
    seen_count = 0;
    skip_count = 0;
    skip_name = NULL;
}

static bool was_skip_called_with(const char* name) {
    for (size_t i = 0; i < skip_count && i < MAX_SEEN; i++) {
        if (strcmp(skip_calls[i], name) == 0) return true;
    }
    return false;
}

static void check_seen(const char* const* expected, size_t count) {
    CHECK(seen_count == count);
    for (size_t i = 0; i < count && i < seen_count; i++) {
        CHECK_STR_EQ(seen[i], expected[i]);
    }
}

// The example tree from the task, created in deliberately unsorted order.
static bool make_example_tree(void) {
    return make_dir("tree")
        && write_text("tree/file_3.c", "3")
        && make_dir("tree/dir_2")
        && make_dir("tree/dir_2/dir_4")          // empty directory
        && make_dir("tree/dir_2/dir_3")
        && write_text("tree/dir_2/dir_3/file_7.txt", "7")
        && write_text("tree/dir_2/file_6.h", "6")
        && write_text("tree/file_1.txt", "1")
        && make_dir("tree/dir_1")
        && write_text("tree/dir_1/file_5.sh", "5")
        && write_text("tree/dir_1/file_4", "4")
        && write_text("tree/file_2.png", "2");
}

// Files of a directory first (sorted), then its subdirectories, level by level.
static void test_order(void) {
    const char* expected[] = {
        "tree/file_1.txt",
        "tree/file_2.png",
        "tree/file_3.c",
        "tree/dir_1/file_4",
        "tree/dir_1/file_5.sh",
        "tree/dir_2/file_6.h",
        "tree/dir_2/dir_3/file_7.txt",
    };
    iterarive_walk("tree", collect, NULL);
    check_seen(expected, sizeof(expected) / sizeof(*expected));
    reset_seen();
}

// A trailing '/' in the root does not produce "//" in paths.
static void test_trailing_separator(void) {
    iterarive_walk("tree/", collect, NULL);
    CHECK(seen_count == 7);
    if (seen_count > 0) CHECK_STR_EQ(seen[0], "tree/file_1.txt");
    for (size_t i = 0; i < seen_count && i < MAX_SEEN; i++) {
        CHECK(strstr(seen[i], "//") == NULL);
    }
    reset_seen();
}

static void test_skip_hook(void) {
    // skip gets bare names, never paths, and never the root itself
    iterarive_walk("tree", collect, skip_hook);
    CHECK(seen_count == 7);
    CHECK(skip_count == 11);   // 7 files + 4 directories
    for (size_t i = 0; i < skip_count && i < MAX_SEEN; i++) {
        CHECK(strchr(skip_calls[i], '/') == NULL);
        CHECK(strcmp(skip_calls[i], "tree") != 0);
    }
    reset_seen();

    // skipping a file drops only that file
    skip_name = "file_4";
    iterarive_walk("tree", collect, skip_hook);
    CHECK(seen_count == 6);
    for (size_t i = 0; i < seen_count && i < MAX_SEEN; i++) {
        CHECK(strstr(seen[i], "file_4") == NULL);
    }
    reset_seen();

    // skipping a directory drops its whole subtree without opening it
    skip_name = "dir_2";
    const char* expected[] = {
        "tree/file_1.txt",
        "tree/file_2.png",
        "tree/file_3.c",
        "tree/dir_1/file_4",
        "tree/dir_1/file_5.sh",
    };
    iterarive_walk("tree", collect, skip_hook);
    check_seen(expected, sizeof(expected) / sizeof(*expected));
    CHECK(!was_skip_called_with("file_6.h"));
    CHECK(!was_skip_called_with("dir_3"));
    reset_seen();
}

static void test_empty_and_invalid_roots(void) {
    CHECK(make_dir("empty"));
    iterarive_walk("empty", collect, NULL);
    CHECK(seen_count == 0);
    reset_seen();

    // errors are reported to stderr, nothing is passed to the callback
    iterarive_walk("does_not_exist", collect, NULL);
    CHECK(seen_count == 0);
    iterarive_walk("tree/file_1.txt", collect, NULL);
    CHECK(seen_count == 0);
    reset_seen();
}

static void test_is_directory_and_file(void) {
    CHECK(is_directory("tree"));
    CHECK(is_directory("tree/dir_2/dir_4"));
    CHECK(!is_directory("tree/file_1.txt"));
    CHECK(!is_directory("does_not_exist"));

    CHECK(is_regular_file("tree/file_1.txt"));
    CHECK(!is_regular_file("tree"));
    CHECK(!is_regular_file("does_not_exist"));
}

#ifndef _WIN32
static void test_symlinks_and_special_files(void) {
    CHECK(make_dir("links"));
    CHECK(write_text("links/real.txt", "real"));
    CHECK(make_dir("links/sub"));
    CHECK(write_text("links/sub/inner.txt", "inner"));
    CHECK(symlink("real.txt", "links/file_link") == 0);   // to a file: reported
    CHECK(symlink("sub", "links/dir_link") == 0);         // to a dir: not followed
    CHECK(symlink("..", "links/sub/loop") == 0);          // cycle: must terminate
    CHECK(symlink("missing", "links/dangling") == 0);     // broken: error, skipped

    // not a regular file; some file systems (e.g. Windows drives mounted
    // into WSL) cannot hold FIFOs, then this part is skipped
    bool have_fifo = mkfifo("links/fifo", 0644) == 0;
    if (!have_fifo) printf("FIFOs not supported here: FIFO checks skipped\n");

    const char* expected[] = {
        "links/file_link",
        "links/real.txt",
        "links/sub/inner.txt",
    };
    iterarive_walk("links", collect, NULL);
    check_seen(expected, sizeof(expected) / sizeof(*expected));
    reset_seen();

    CHECK(is_directory("links/dir_link"));       // stat() follows links
    CHECK(is_regular_file("links/file_link"));
    if (have_fifo) CHECK(!is_regular_file("links/fifo"));
    CHECK(!is_regular_file("links/dangling"));
}
#endif

int main(int argc, char** argv) {
    if (!enter_work_dir(argc, argv)) return 1;
    if (!make_example_tree()) return 1;

    test_order();
    test_trailing_separator();
    test_skip_hook();
    test_empty_and_invalid_roots();
    test_is_directory_and_file();
#ifndef _WIN32
    test_symlinks_and_special_files();
#endif
    return TEST_SUMMARY();
}
