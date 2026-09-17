#ifndef CRYPTO_PRIMITIVES_ARIA_H
#define CRYPTO_PRIMITIVES_ARIA_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    uint8_t enc_round_keys[17][16];
    uint8_t dec_round_keys[17][16];
} aria_ctx;

extern const block_cipher aria128_block_cipher;
extern const block_cipher aria192_block_cipher;
extern const block_cipher aria256_block_cipher;

void aria128_expand_key(void *ctx, const uint8_t *master_key);
void aria192_expand_key(void *ctx, const uint8_t *master_key);
void aria256_expand_key(void *ctx, const uint8_t *master_key);
void aria_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void aria_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void aria_clear(void *ctx);

#endif
