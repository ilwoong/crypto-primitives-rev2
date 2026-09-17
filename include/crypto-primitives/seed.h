#ifndef CRYPTO_PRIMITIVES_SEED_H
#define CRYPTO_PRIMITIVES_SEED_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t round_keys[32];
} seed_ctx;

extern const block_cipher seed_block_cipher;

void seed_expand_key(void *ctx, const uint8_t *master_key);
void seed_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void seed_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void seed_clear(void *ctx);

#endif
