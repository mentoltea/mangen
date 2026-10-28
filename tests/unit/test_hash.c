// CRC-32 implementation: reference vectors, incremental hashing, hex output.
// Expected values come from zlib (Python zlib.crc32).

#include <stdlib.h>

#include "hash.h"
#include "test.h"

// Hashes data in one call and returns its hex form in buf.
static const char* hex_of(const void* data, size_t size, char* buf) {
    struct HashCtx ctx;
    struct Hash hash;
    hash_init(&ctx);
    hash_update(&ctx, data, size);
    hash_final(&ctx, &hash);
    hash_to_hex(&hash, buf);
    return buf;
}

static void test_reference_vectors(void) {
    char buf[HASH_HEX_SIZE];
    CHECK_STR_EQ(hex_of("", 0, buf), "00000000");
    CHECK_STR_EQ(hex_of("a", 1, buf), "E8B7BE43");
    CHECK_STR_EQ(hex_of("abc", 3, buf), "352441C2");
    // the standard CRC-32 "check" value
    CHECK_STR_EQ(hex_of("123456789", 9, buf), "CBF43926");
    CHECK_STR_EQ(hex_of("The quick brown fox jumps over the lazy dog", 43, buf), "414FA339");
}

static void test_binary_data(void) {
    char buf[HASH_HEX_SIZE];
    const unsigned char zeros[3] = {0, 0, 0};
    const unsigned char ones[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    // NUL bytes must be hashed, not treated as a string terminator
    CHECK_STR_EQ(hex_of(zeros, sizeof(zeros), buf), "FF41D912");
    CHECK_STR_EQ(hex_of(ones, sizeof(ones), buf), "FFFFFFFF");
}

static void test_digest_size(void) {
    struct HashCtx ctx;
    struct Hash hash;
    hash_init(&ctx);
    hash_final(&ctx, &hash);
    CHECK(hash.size == 4);
    CHECK(hash.size <= HASH_MAX_SIZE);
}

// The result must not depend on how the data is split into chunks.
static void test_incremental(void) {
    enum { SIZE = 300000 };
    unsigned char* data = malloc(SIZE);
    CHECK(data != NULL);
    if (!data) return;

    unsigned int seed = 12345;
    for (size_t i = 0; i < SIZE; i++) {
        seed = seed * 1103515245u + 12345u;
        data[i] = (unsigned char)(seed >> 16);
    }

    char whole[HASH_HEX_SIZE];
    hex_of(data, SIZE, whole);

    const size_t chunk_sizes[] = {1, 7, 4096, 65536, 99999};
    for (size_t c = 0; c < sizeof(chunk_sizes) / sizeof(*chunk_sizes); c++) {
        struct HashCtx ctx;
        struct Hash hash;
        hash_init(&ctx);
        for (size_t off = 0; off < SIZE; off += chunk_sizes[c]) {
            size_t n = SIZE - off < chunk_sizes[c] ? SIZE - off : chunk_sizes[c];
            hash_update(&ctx, data + off, n);
        }
        hash_update(&ctx, data, 0);  // empty updates change nothing
        hash_final(&ctx, &hash);

        char chunked[HASH_HEX_SIZE];
        hash_to_hex(&hash, chunked);
        CHECK_STR_EQ(chunked, whole);
    }
    free(data);
}

// A context can be reused after hash_init().
static void test_reinit(void) {
    struct HashCtx ctx;
    struct Hash hash;
    char buf[HASH_HEX_SIZE];

    hash_init(&ctx);
    hash_update(&ctx, "garbage", 7);
    hash_init(&ctx);
    hash_update(&ctx, "abc", 3);
    hash_final(&ctx, &hash);
    hash_to_hex(&hash, buf);
    CHECK_STR_EQ(buf, "352441C2");
}

static void test_hex_format(void) {
    struct Hash hash = {.bytes = {0x00, 0x0A, 0xBC, 0xFF, 0x12}, .size = 5};
    char buf[HASH_HEX_SIZE];
    memset(buf, 'x', sizeof(buf));

    hash_to_hex(&hash, buf);
    CHECK_STR_EQ(buf, "000ABCFF12");   // uppercase, leading zeros kept, terminated

    hash.size = 0;
    hash_to_hex(&hash, buf);
    CHECK_STR_EQ(buf, "");
}

int main(void) {
    test_reference_vectors();
    test_binary_data();
    test_digest_size();
    test_incremental();
    test_reinit();
    test_hex_format();
    return TEST_SUMMARY();
}
