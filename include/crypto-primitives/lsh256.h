#ifndef CRYPTO_PRIMITIVES_LSH256_H
#define CRYPTO_PRIMITIVES_LSH256_H

#include "message-digest.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t block_index;
    uint8_t block[128];
    uint32_t cv[16];
    uint32_t tcv[16];
    uint32_t msg[16 * (26 + 1)];
} lsh256_ctx;

extern const message_digest lsh256_message_digest;

void lsh256_init(void *ctx);
void lsh256_update(void *ctx, const uint8_t *in, size_t len);
void lsh256_final(void *ctx, uint8_t *out);

#endif
