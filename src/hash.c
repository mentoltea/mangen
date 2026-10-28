// CRC-32 (IEEE 802.3)

#include "hash.h"

#include <stdbool.h>

#define CRC32_POLY 0xEDB88320u
#define CRC32_SIZE 4

static uint32_t crc_table[256];
static bool crc_table_ready = false;

static void init_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int bit = 0; bit < 8; bit++) {
            c = (c & 1) ? (c >> 1) ^ CRC32_POLY : (c >> 1);
        }
        crc_table[i] = c;
    }
    crc_table_ready = true;
}

void hash_init(struct HashCtx* ctx) {
    if (!crc_table_ready) init_crc_table();
    ctx->state = 0xFFFFFFFFu;
}

void hash_update(struct HashCtx* ctx, const void* data, size_t size) {
    const uint8_t* bytes = data;
    uint32_t crc = ctx->state;
    for (size_t i = 0; i < size; i++) {
        crc = crc_table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
    }
    ctx->state = crc;
}

void hash_final(struct HashCtx* ctx, struct Hash* out) {
    uint32_t crc = ~ctx->state;

    out->size = CRC32_SIZE;
    out->bytes[0] = (uint8_t)(crc >> 24);
    out->bytes[1] = (uint8_t)(crc >> 16);
    out->bytes[2] = (uint8_t)(crc >> 8);
    out->bytes[3] = (uint8_t)crc;
}

void hash_to_hex(const struct Hash* hash, char* buf) {
    static const char digits[] = "0123456789ABCDEF";
    for (size_t i = 0; i < hash->size; i++) {
        buf[2 * i]     = digits[hash->bytes[i] >> 4];
        buf[2 * i + 1] = digits[hash->bytes[i] & 0x0F];
    }
    buf[2 * hash->size] = '\0';
}
