#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>

#define HASH_MAX_SIZE 32

// two hex digits per byte + '\0'.
#define HASH_HEX_SIZE (2 * HASH_MAX_SIZE + 1)

struct Hash {
    uint8_t bytes[HASH_MAX_SIZE];
    size_t size;
};

struct HashCtx {
    uint32_t state;
};

void hash_init(struct HashCtx* ctx);
void hash_update(struct HashCtx* ctx, const void* data, size_t size);
void hash_final(struct HashCtx* ctx, struct Hash* out);


void hash_to_hex(const struct Hash* hash, char* buf);

#endif // HASH_H
HASH_H