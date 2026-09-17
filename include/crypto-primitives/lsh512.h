#ifndef CRYPTO_PRIMITIVES_LSH512_H
#define CRYPTO_PRIMITIVES_LSH512_H

#include "message-digest.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t block_index;
    uint8_t block[256];
    uint64_t cv[16];
    uint64_t tcv[16];
    uint64_t msg[16 * (28 + 1)];
} lsh512_ctx;

extern const message_digest lsh512_message_digest;

void lsh512_init(void *ctx);
void lsh512_update(void *ctx, const uint8_t *in, size_t len);
void lsh512_final(void *ctx, uint8_t *out);

#endif
