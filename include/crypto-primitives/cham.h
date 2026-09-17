#ifndef CRYPTO_PRIMITIVES_CHAM_H
#define CRYPTO_PRIMITIVES_CHAM_H

#include "cipher.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t rounds;
    size_t num_round_keys;
    uint32_t round_keys[16];
} cham_ctx;

extern const block_cipher cham64_128_block_cipher;
extern const block_cipher cham128_128_block_cipher;
extern const block_cipher cham128_256_block_cipher;

void cham64_128_expand_key(void *ctx, const uint8_t *master_key);
void cham128_128_expand_key(void *ctx, const uint8_t *master_key);
void cham128_256_expand_key(void *ctx, const uint8_t *master_key);

void cham64_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void cham64_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void cham128_encrypt(void *ctx, uint8_t *out, const uint8_t *in);
void cham128_decrypt(void *ctx, uint8_t *out, const uint8_t *in);
void cham_clear(void *ctx);

#endif
