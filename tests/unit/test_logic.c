// File hashing (hash_file) and the list of collected pairs
// (hash_file_and_store, get_pairs, reset_pairs).

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>

#include "fs_util.h"
#include "logic.h"
#include "test.h"

// hash_file() result as hex, or "FAILED" if it returned false.
static const char* file_hex(const char* path, char* buf) {
    struct Hash hash;
    if (!hash_file(path, &hash)) return "FAILED";
    hash_to_hex(&hash, buf);
    return buf;
}

static void test_hash_file(void) {
    char buf[HASH_HEX_SIZE];

    CHECK(write_text("abc.txt", "abc"));
    CHECK_STR_EQ(file_hex("abc.txt", buf), "352441C2");

    CHECK(write_text("empty", ""));
    CHECK_STR_EQ(file_hex("empty", buf), "00000000");

    // binary content with NUL bytes
    const unsigned char zeros[3] = {0, 0, 0};
    CHECK(write_file("zeros.bin", zeros, sizeof(zeros)));
    CHECK_STR_EQ(file_hex("zeros.bin", buf), "FF41D912");

    // 200 KiB: spans several 64 KiB read chunks; value from zlib
    const char* pattern = "0123456789abcdef";
    size_t size = 16 * 12800;
    char* big = malloc(size);
    CHECK(big != NULL);
    if (big) {
        for (size_t i = 0; i < size; i++) big[i] = pattern[i % 16];
        CHECK(write_file("big.bin", big, size));
        CHECK_STR_EQ(file_hex("big.bin", buf), "65DEC7D0");
        free(big);
    }
}

static void test_hash_file_errors(void) {
    char buf[HASH_HEX_SIZE];
    CHECK_STR_EQ(file_hex("does_not_exist", buf), "FAILED");

    // a directory cannot be read as a file
    CHECK(make_dir("a_dir"));
#ifndef _WIN32
    CHECK_STR_EQ(file_hex("a_dir", buf), "FAILED");

    // Permissions are not enforced for root and are ignored by some file
    // systems (e.g. Windows drives mounted into WSL), so first make sure
    // the file really became unreadable.
    CHECK(write_text("secret", "secret"));
    CHECK(chmod("secret", 0) == 0);
    FILE* probe = fopen("secret", "rb");
    if (probe == NULL) {
        CHECK_STR_EQ(file_hex("secret", buf), "FAILED");
    } else {
        fclose(probe);
        printf("permissions not enforced here: unreadable file check skipped\n");
    }
    chmod("secret", 0644);
#endif
}

static void test_pairs(void) {
    reset_pairs();
    CHECK(get_pairs().count == 0);

    CHECK(write_text("one", "a"));
    CHECK(write_text("two", "abc"));

    // the stored path is a copy, independent of the caller's buffer
    char path[16];
    strcpy(path, "one");
    hash_file_and_store(path);
    strcpy(path, "XXX");

    hash_file_and_store("does_not_exist");   // unreadable: not stored
    hash_file_and_store("two");

    struct Pairs pairs = get_pairs();
    CHECK(pairs.count == 2);
    if (pairs.count == 2) {
        char buf[HASH_HEX_SIZE];
        CHECK_STR_EQ(pairs.items[0].path, "one");
        hash_to_hex(&pairs.items[0].hash, buf);
        CHECK_STR_EQ(buf, "E8B7BE43");

        CHECK_STR_EQ(pairs.items[1].path, "two");
        hash_to_hex(&pairs.items[1].hash, buf);
        CHECK_STR_EQ(buf, "352441C2");
    }

    reset_pairs();
    CHECK(get_pairs().count == 0);
    CHECK(get_pairs().items == NULL);

    // usable again after a reset; enough items to make the array grow
    for (int i = 0; i < 25; i++) hash_file_and_store("one");
    CHECK(get_pairs().count == 25);
    reset_pairs();
    CHECK(get_pairs().count == 0);
}

int main(int argc, char** argv) {
    if (!enter_work_dir(argc, argv)) return 1;

    test_hash_file();
    test_hash_file_errors();
    test_pairs();
    return TEST_SUMMARY();
}
